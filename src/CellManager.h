#pragma once
#include "ofMain.h"
#include "World.h"

// A struct for maintaining the state of a single cell.
struct Cell
{
public:
    // The mesh containing the terrain geometry for the cell.
    ofMesh terrainMesh {};

    // The corner defining the mesh's location in world space.
    glm::vec2 startPos {};

    // Set to true while the mesh is loading so that it's not rendered mid-load.
    bool loading { false };

    // Set to false while the cell is inactive so that it's not rendered.
    bool live { false };
};

// A template class for managing partial terrain meshes, 
// automatically loading and unloading cells as they go in and out of draw range.
template<unsigned int CELL_PAIRS_PER_DIMENSION>
class CellManager
{
public:
    CellManager(const World& world, unsigned int cellSize) 
        : world { world }, cellSize { cellSize }
    {
    }

    // Don't support copy constructor or copy assignment operator.
    CellManager(const CellManager& c) = delete;
    CellManager& operator= (const CellManager& c) = delete;

    // This function should be called in your ofApp::setup() function.  
    // Pass in whatever position you want the loaded terrain to be centered around.
    void initializeForPosition(glm::vec3 position)
    {
        // Calculate the size of a cell in world coordinates.
        glm::vec2 scaledCellSize { getScaledCellSize() };

        // The range of loaded cells should be centered on the player.
        glm::vec2 cellGridMidIndices { glm::round(glm::vec2(position.x, position.z) / scaledCellSize) };

        // Calculate the actual start position of the first cell in the loaded grid.
        cellGridStartPos = (cellGridMidIndices - glm::vec2(CELL_PAIRS_PER_DIMENSION)) * scaledCellSize;

        // Load each cell.
        unsigned int bufferIndex { 0 };
        for (unsigned int i { 0 }; i < 2 * CELL_PAIRS_PER_DIMENSION; i++)
        {
            for (unsigned int j { 0 }; j < 2 * CELL_PAIRS_PER_DIMENSION; j++)
            {
                initCell(cellBuffer[bufferIndex], (cellGridStartPos + glm::vec2(i, j) * scaledCellSize));
                bufferIndex++;
            }
        }
    }

    // This function should be called in your ofApp::update() function to unload cells that have gotten to be far away
    // and request new cells that have gotten closer.  No meshes are actually loaded in this function;
    // call processLoadQueue to actually load the meshes.
    void optimizeForPosition(glm::vec3 position)
    {
        // Calculate the size of a cell in world coordinates.
        glm::vec2 scaledCellSize { getScaledCellSize() };

        // Calculate a lower bound (in each dimension) on where the lower grid can start.
        glm::vec2 minGridStartIndices { glm::ceil(glm::vec2(position.x, position.z) / scaledCellSize) - glm::vec2(CELL_PAIRS_PER_DIMENSION) };

        // Calculate the actual start position of the first cell in the loaded grid at the lower bound in each dimension.
        glm::vec2 minGridStartPos { minGridStartIndices * scaledCellSize };

        // THe actual start position of the first cell in the loaded grid at the upper bound in each dimension.
        glm::vec2 maxGridStartPos { minGridStartPos + scaledCellSize };

        // Only move the grid of loaded cells if it's outside of the lower / upper bounds.
        // This ensures that unnecessary loading doesn't occur.
        glm::vec2 newGridStartPos { clamp(cellGridStartPos, minGridStartPos, maxGridStartPos) };

        // Only do something if the grid of loaded cells needs to change.
        if (newGridStartPos != cellGridStartPos)
        {
            int bufferIndex { 0 };

            if (newGridStartPos.x > cellGridStartPos.x)
            {
                // Add new cells to the right of the old active region
                for (unsigned int j { 0 }; j < 2 * CELL_PAIRS_PER_DIMENSION; j++)
                {
                    cellLoadQueue.push(newGridStartPos + glm::vec2(2 * CELL_PAIRS_PER_DIMENSION - 1, j) * scaledCellSize);
                }
            }
            else if (newGridStartPos.x < cellGridStartPos.x)
            {
                // Add new cells to the left of the old active region
                for (unsigned int j { 0 }; j < 2 * CELL_PAIRS_PER_DIMENSION; j++)
                {
                    cellLoadQueue.push(newGridStartPos + glm::vec2(0, j * scaledCellSize.y));
                }
            }

            if (newGridStartPos.y > cellGridStartPos.y)
            {
                // Add new cells above the old active region
                // Do the top left corner only if we didn't just add an entire left-hand column of cells in the preceding code
                if (newGridStartPos.x >= cellGridStartPos.x)
                {
                    cellLoadQueue.push(newGridStartPos + glm::vec2(0, (2 * CELL_PAIRS_PER_DIMENSION - 1) * scaledCellSize.y));
                }

                // Add the top middle cells
                for (unsigned int i { 1 }; i < 2 * CELL_PAIRS_PER_DIMENSION - 1; i++)
                {
                    cellLoadQueue.push(newGridStartPos + glm::vec2(i, 2 * CELL_PAIRS_PER_DIMENSION - 1) * scaledCellSize);
                }

                // Do the top right corner only if we didn't just add an entire right-hand column of cells in the preceding code
                if (newGridStartPos.x <= cellGridStartPos.x)
                {
                    cellLoadQueue.push(newGridStartPos + glm::vec2(2 * CELL_PAIRS_PER_DIMENSION - 1, 2 * CELL_PAIRS_PER_DIMENSION - 1) * scaledCellSize);
                }
            }
            else if (newGridStartPos.y < cellGridStartPos.y)
            {
                // Add new cells below the old active region
                // Do the bottom left corner only if we didn't just add an entire left-hand column of cells in the preceding code
                if (newGridStartPos.x >= cellGridStartPos.x)
                {
                    cellLoadQueue.push(newGridStartPos);
                }

                // Add the bottom middle cells
                for (unsigned int i { 1 }; i < 2 * CELL_PAIRS_PER_DIMENSION - 1; i++)
                {
                    cellLoadQueue.push(newGridStartPos + glm::vec2(i * scaledCellSize.x, 0));
                }

                // Do the bottom right corner only if we didn't just add an entire right-hand column of cells in the preceding code
                if (newGridStartPos.x <= cellGridStartPos.x)
                {
                    cellLoadQueue.push(newGridStartPos + glm::vec2((2 * CELL_PAIRS_PER_DIMENSION - 1) * scaledCellSize.x, 0));
                }
            }

            // Update the starting coordinates of the new grid of loaded cells.
            cellGridStartPos = newGridStartPos;
        }
    }

    // This function would also be called in your ofApp::update() function.  
    // This function is where the terrain meshes actually get created.
    void processLoadQueue()
    {
        if (!cellLoadQueue.empty())
        {
            // Calculate the size of a cell in world coordinates.
            glm::vec2 scaledCellSize { getScaledCellSize() };

            // Deactivate cells that are now out of range.
            //cout << "Active cells: " << endl;
            for (Cell& cell : cellBuffer)
            {
                if (isCellDistant(cell.startPos))
                {
                    cell.live = false;
                }
            }

            unsigned int bufferIndex { 0 };

            // Keep processing until there aren't any available cells in the buffer, or there are not cell requests left to process.
            while (bufferIndex < CELL_BUFFER_SIZE && !cellLoadQueue.empty())
            {
                // Find the next unused cell in the buffer.
                while (bufferIndex < CELL_BUFFER_SIZE && (cellBuffer[bufferIndex].live || cellBuffer[bufferIndex].loading))
                {
                    bufferIndex++;
                }

                if (bufferIndex < CELL_BUFFER_SIZE)
                {
                    if (!isCellDistant(cellLoadQueue.front())
                        && !isCellDuplicate(cellLoadQueue.front(), glm::min(scaledCellSize.x, scaledCellSize.y) * 0.125f)) // Make sure we still want the cell
                    {
                        // Load the next requested cell.
                        initCell(cellBuffer[bufferIndex], cellLoadQueue.front());
                    }

                    cellLoadQueue.pop();
                }
            }
        }
    }

    // This function iterates over all the available cells and draws all of them that are within the draw 
    // distance from the current camera position. This should be called from your ofApp::draw() function.  
    // The draw distance should be the same as the far plane from your projection matrix.
    void drawActiveCells(glm::vec3 camPosition, float drawDistance)
    {
        // Calculate the size of a cell in world coordinates.
        glm::vec2 scaledCellSize { getScaledCellSize() };

        // Calculate an appropriate threshold for deciding if cells are too far away to draw.
        float threshold = drawDistance + glm::max(scaledCellSize.x, scaledCellSize.y) * glm::sqrt(0.5f);

        for (Cell& cell : cellBuffer)
        {
            // Make sure the cell is live/active and check the distance from the cell center to the camera position
            if (cell.live && distance(glm::vec2(camPosition.x, camPosition.z), cell.startPos + scaledCellSize * 0.5f) < threshold)
            {
                // Draw the cell.
                cell.terrainMesh.draw();
            }
        }
    }

private:
    // The maximum number of cells that can be currently loaded at once.
    const static unsigned int CELL_BUFFER_SIZE { 4 * CELL_PAIRS_PER_DIMENSION * CELL_PAIRS_PER_DIMENSION };

    // The buffer of loaded cells.
    Cell cellBuffer[CELL_BUFFER_SIZE] {};

    // A reference to the world associated with this cell manager.
    const World& world;

    // The size of each cell (assumed to be square).
    unsigned int cellSize;

    // The "first" cell that is currently loaded; the corner of the rectangle of loaded cells.
    glm::vec2 cellGridStartPos;

    // A queue containing the corners of cells that need to be loaded.
    std::queue<glm::vec2> cellLoadQueue {};

    glm::vec2 getScaledCellSize() const
    {
        // The dimensions (in pixels) of the heightmap.
        glm::vec2 heightmapSize { world.heightmap->getWidth() - 1, world.heightmap->getHeight() - 1 };

        // The specified size of the heightmap (in world coordinates).
        glm::vec2 worldHeightmapScale { glm::vec2(world.dimensions.x, world.dimensions.z) };

        // Convert the cell size (defined in terms of heightmap pixels) to world coordinates.
        return worldHeightmapScale * cellSize / heightmapSize;
    }

    bool isCellDistant(glm::vec2 cellStartPos)
    {
        glm::vec2 scaledCellSize { getScaledCellSize() };

        // Distant cells are outside of the rectangle of cells currently loaded.
        return cellStartPos.x < cellGridStartPos.x - scaledCellSize.x * 0.125f // subtract a little extra to account for round-off error
            || cellStartPos.y < cellGridStartPos.y - scaledCellSize.y * 0.125f
            || cellStartPos.x + scaledCellSize.x > cellGridStartPos.x + scaledCellSize.x * (2 * CELL_PAIRS_PER_DIMENSION + 0.125f)
            || cellStartPos.y + scaledCellSize.y > cellGridStartPos.y + scaledCellSize.y * (2 * CELL_PAIRS_PER_DIMENSION + 0.125f);
    }

    bool isCellDuplicate(glm::vec2 cellStartPos, float tolerance)
    {
        for (Cell& otherCell : cellBuffer)
        {
            // If two cells' start position is within a certain tolerance, they are considered duplicates.
            if (distance(otherCell.startPos, cellStartPos) < tolerance)
            {
                return true;
            }
        }

        return false;
    }
    
    void initCell(Cell& cell, glm::vec2 startPos)
    {
        // Set cell's starting position, it is current loading and not yet live.
        cell.startPos = startPos;
        cell.live = false;
        cell.loading = true;

        // After unscaling, should range between (0, 0, 0) and (1, 1, 1)
        glm::vec3 unscaledStartPos { glm::vec3(startPos.x, 0, startPos.y) / world.dimensions };

        // Remap to the resolution of the heightmap and round to the nearest integer
        glm::uvec2 startIndices { round(glm::vec2(
            unscaledStartPos.x * (world.heightmap->getWidth() - 1), 
            unscaledStartPos.z * (world.heightmap->getHeight() - 1))) };

        // Clear the old terrain mesh and rebuild it for the current cell.
        cell.terrainMesh.clear();
        world.buildMeshForTerrainCell(cell.terrainMesh, startIndices, glm::uvec2(cellSize, cellSize));

        // Once the cell has been successfully loaded, make it live.
        cell.loading = false;
        cell.live = true;
    }
};

#pragma once

#include "ofURLFileLoader.h"
#include "uriparser/Uri.h"
#include "ofTexture.h"
#include "ofImage.h"

class ofxCubemap
{
public:
	ofxCubemap();
	~ofxCubemap();
	ofxCubemap(const ofxCubemap& other) = delete;
	ofxCubemap& operator=(const ofxCubemap& other) = delete;

	bool load(const std::filesystem::path& front,
			  const std::filesystem::path& back,
			  const std::filesystem::path& right,
			  const std::filesystem::path& left,
			  const std::filesystem::path& top,
			  const std::filesystem::path& bottom);

	ofTexture& getTexture();
	const ofTexture& getTexture() const;

private:
	ofTexture textureData;
	unsigned int glTexId;
	ofImage images[6];

};

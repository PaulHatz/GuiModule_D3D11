#include "main.h"

FontTexture fontTexture = {};

TexChild::TexChild(float fontSize, ID3D11ShaderResourceView *view, int width, int height, int bitmap_left, int bitmap_top, int advance_x, int advance_y)
{
	this->fontSize = fontSize;
	this->shaderResourceView = view;
	this->next = nullptr;

	this->glyphInfo = {};
	this->glyphInfo.width = width;
	this->glyphInfo.height = height;
	this->glyphInfo.bitmap_left = bitmap_left;
	this->glyphInfo.bitmap_top = bitmap_top;
	this->glyphInfo.advance_x = advance_x;
	this->glyphInfo.advance_y = advance_y;
}

ID3D11ShaderResourceView *getCachedShaderResourceViewForChar(BYTE c, float fontSize)
{
	auto child = fontTexture.codes[c];

	while (child != nullptr) {

		if (child->fontSize == fontSize) {
			return child->shaderResourceView;
		}
		child = child->next;
	}

	return nullptr;
}

TexChild *getCachedTextInfoForChar(BYTE c, float fontSize)
{
	auto child = fontTexture.codes[c];

	while (child != nullptr) {

		if (child->fontSize == fontSize) {
			return child;
		}
		child = child->next;
	}

	return nullptr;
}

TexChild *addTextInfoToCache(BYTE c, float fontSize, ID3D11ShaderResourceView *view, int width, int height, int bitmap_left, int bitmap_top, int advance_x, int advance_y)
{
	auto child = fontTexture.codes[c];

	if (!child) {
		fontTexture.codes[c] = new TexChild(fontSize, view, width, height, bitmap_left, bitmap_top, advance_x, advance_y);
		return fontTexture.codes[c];
	}

	while (child != nullptr) {
		if (child->next)
			child = child->next;
		else break;

	}
	child->next = new TexChild(fontSize, view, width, height, bitmap_left, bitmap_top, advance_x, advance_y);

	return child->next;
}


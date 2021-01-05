#pragma once

struct GlyphInfo
{
	int bitmap_left;
	int bitmap_top;
	int width;
	int height;
	int advance_x;
	int advance_y;
};

struct TexChild
{
	float fontSize;
	TexChild *next;
	ID3D11ShaderResourceView *shaderResourceView;
	GlyphInfo glyphInfo;

	TexChild(float fontSize, ID3D11ShaderResourceView *view, int width, int height, int bitmap_left, int bitmap_top, int advance_x, int advance_y);
};

struct FontTexture
{
	TexChild *codes[255];
};

ID3D11ShaderResourceView *getCachedShaderResourceViewForChar(BYTE c, float fontSize);
TexChild *getCachedTextInfoForChar(BYTE c, float fontSize);
TexChild *addTextInfoToCache(BYTE c, float fontSize, ID3D11ShaderResourceView *view, int width, int height, int bitmap_left, int bitmap_top, int advance_x, int advance_y);

#pragma once


class Font
{
private:
	FT_Face face;
	static FT_Library library;
	float fontSize;

public:
	
	static void initFreeType();
	static FT_Library getFontLibrary();

	FT_Face getFace();

	void setFontSize(float fontSize);
	float getFontSize();

	Font(const char *path);
	Font(unsigned char *buffer, size_t length);

};


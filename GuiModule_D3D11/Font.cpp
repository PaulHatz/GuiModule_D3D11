#include "main.h"

FT_Library Font::library;

void Font::initFreeType()
{
	if (FT_Init_FreeType(&library)) {
		printf("Error: FT_Init_FreeType\n");
	}
}

FT_Library Font::getFontLibrary()
{
	return library;
}

FT_Face Font::getFace()
{
	return this->face;
}

void Font::setFontSize(float fontSize)
{
	if (this->fontSize == fontSize)
		return;

	this->fontSize = fontSize;
	
	FT_Set_Pixel_Sizes(this->face, 0, FT_UInt(fontSize));
}

float Font::getFontSize()
{
	return this->fontSize;
}

Font::Font(unsigned char *data, size_t length)
{
	if (FT_New_Memory_Face(getFontLibrary(), data, length, 0, &face)) {
		printf("Error: FT_New_Memory_Face\n");

		exit(0);
	}
}

Font::Font(const char *path)
{
	if (FT_New_Face(getFontLibrary(), path, 0, &face)) {
		printf("Error: FT_New_Face\n");

		exit(0);
	}

	this->fontSize = 12;

	FT_Set_Pixel_Sizes(this->face, 0, FT_UInt(fontSize));

}
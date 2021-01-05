<h1>GuiModule_D3D11</h1>
<p>D3D11 interface for rendering in cppgui</p>


<h2>Example</h2>
<code>
__declspec(dllimport) void __cdecl initializeWindow(WNDPROC WndProc2, void(*DrawingFuntion)());
__declspec(dllimport) void __cdecl drawBox(float x, float y, float w, float h, float r, float g, float b, float a);
__declspec(dllimport) void __cdecl drawText(const char *text, float x, float y, class Font *font, float fontSize, float r, float g, float b, float a);
__declspec(dllexport) class Font *__cdecl getRobotoFont();
__declspec(dllimport) float __cdecl getTextWidth(const char *text, class Font *font, float fontSize);
__declspec(dllimport) float __cdecl getTextHeight(const char *text, Font *font, float fontSize);
__declspec(dllimport) HWND __cdecl getWindowHWND();

void Drawing()
{
	//This will draw the text "Hello World!" with a line under-lining it.
	drawText("Hello World!", 100, 100, getRobotoFont(), 16.f, 0, 0, 0, 255);
	drawBox(100, 116, getTextWidth("Hello World!", getRobotoFont(), 16.f), 2, 0, 0, 0, 255);
}

int main()
{
	initializeWindow(nullptr, Drawing);
	return 0;
}
</code>

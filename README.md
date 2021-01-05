<html>
<head>
	<META NAME="GENERATOR" Content="Microsoft Visual Studio">
</head>
<body>
<h1>GuiModule_D3D11</h1>
<p>D3D11 interface for rendering in cppgui.</p>


<h3>What is this?</h3>
<p>
	This is a C++ library which creates a window, initializes a D3D11 Device, and renders rectangles as well as fonts onto the window. Font rendering is achieved using <a href="https://www.freetype.org/">FreeType</a>.
</p

<p>
	This library was created to be a modular library for <a href="https://github.com/PaulHatz/cppgui">cppgui</a>. which is designed to compile on any platform.
</p>

<h3>How to use this</h3>
<p>
	This library was made to be very easy to use. It handles anything relating to rendering automatically, so usage is very straight forward. In it's current form, there are no additional header files required. The library can be linked by adding this into a C++ project.
	
</p>

<code>#pragma comment(lib, "GuiModule_D3D11.lib")</code>

<p>Once the library is linked, we can import the functions being exported</p>

```
__declspec(dllimport) void __cdecl initializeWindow(WNDPROC WndProc2, void(*DrawingFuntion)());
__declspec(dllimport) void __cdecl drawBox(float x, float y, float w, float h, float r, float g, float b, float a);
__declspec(dllimport) void __cdecl drawText(const char *text, float x, float y, class Font *font, float fontSize, float r, float g, float b, float a);
__declspec(dllexport) class Font *__cdecl getRobotoFont();
__declspec(dllimport) float __cdecl getTextWidth(const char *text, class Font *font, float fontSize);
__declspec(dllimport) float __cdecl getTextHeight(const char *text, Font *font, float fontSize);
__declspec(dllimport) HWND __cdecl getWindowHWND();
```

<p>We can add in our drawing function.</p>

```
void Drawing() {
	//This will draw the text "Hello World!" with a line under-lining it.
	drawText("Hello World!", 100, 100, getRobotoFont(), 16.f, 0, 0, 0, 255);
	drawBox(100, 116, getTextWidth("Hello World!", getRobotoFont(), 16.f), 2, 0, 0, 0, 255);
}
```

<p>Finally, in a thread we must initialize the window.</p>



```
int main() {
	//The initializeWindow function contains a while-loop. 
	//You can continue to use this, or create a new to handle the GUI.
	initializeWindow(nullptr, Drawing);
	
	return 0;
}
```


<h3>Credits</h3>
<ul>
	<li>Paul Hatzievagelou</li>
	<li>Portions of this software are copyright © 2021 The FreeType
    Project (www.freetype.org).  All rights reserved.</li>

</ul>




</body>

</html>

#include "main.h"

ID3D11Device *g_pd3dDevice = NULL;
ID3D11DeviceContext *g_pd3dDeviceContext = NULL;
IDXGISwapChain *g_pSwapChain = NULL;
ID3D11RenderTargetView *g_mainRenderTargetView = NULL;
IDXGIFactory *g_pFactory = NULL;
ID3D11Buffer *tempProjectionBuffer = nullptr;
ID3D11Buffer *g_pVertexConstantBuffer = NULL;
ID3D11BlendState *g_pBlendState = NULL;
ID3D11RasterizerState *g_pRasterizerState = NULL;
ID3D11DepthStencilState *g_pDepthStencilState = NULL;
ID3D11ShaderResourceView *g_pFontTextureView = NULL;

VERTEX_CONSTANT_BUFFER2 const_buffer = {};

Font *roboto;

HWND g_hWnd = 0;

std::vector<D3DElem> d3dElems;

using Vec4 = XMFLOAT4;
using Vec3 = XMFLOAT3;
using Vec2 = XMFLOAT2;

using Color = XMVECTORF32;

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) {
		g_mainRenderTargetView->Release();
		g_mainRenderTargetView = NULL;
	}
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D *pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

	if (pBackBuffer)
		g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);

	pBackBuffer->Release();
}

bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return false;

	CreateRenderTarget();

	return true;
}

WNDPROC WndProc2_Client = nullptr;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) {
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			CreateRenderTarget();

		}
		return 0;

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}

	if (WndProc2_Client)
		WndProc2_Client(hWnd, msg, wParam, lParam);

	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

bool ImplDX11_Init(ID3D11Device *device, ID3D11DeviceContext *device_context)
{
	// Get factory from device
	IDXGIDevice *pDXGIDevice = NULL;
	IDXGIAdapter *pDXGIAdapter = NULL;
	IDXGIFactory *pFactory = NULL;

	if (device->QueryInterface(IID_PPV_ARGS(&pDXGIDevice)) == S_OK)
		if (pDXGIDevice->GetParent(IID_PPV_ARGS(&pDXGIAdapter)) == S_OK)
			if (pDXGIAdapter->GetParent(IID_PPV_ARGS(&pFactory)) == S_OK)
			{
				g_pd3dDevice = device;
				g_pd3dDeviceContext = device_context;
				g_pFactory = pFactory;
			}
	if (pDXGIDevice) pDXGIDevice->Release();
	if (pDXGIAdapter) pDXGIAdapter->Release();
	g_pd3dDevice->AddRef();
	g_pd3dDeviceContext->AddRef();

	return true;
}

void setupViewPort(HWND hWnd)
{
	D3D11_VIEWPORT viewport = { 0 };

	RECT rect = { 0 };
	GetClientRect(hWnd, &rect);

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = float(rect.right - rect.left);
	viewport.Height = float(rect.bottom - rect.top);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	float L = float(rect.left);
	float R = float(rect.right);
	float T = float(rect.top);
	float B = float(rect.bottom);

	g_pd3dDeviceContext->RSSetViewports(1, &viewport);

	{
		D3D11_MAPPED_SUBRESOURCE mapped_resource;
		if (g_pd3dDeviceContext->Map(g_pVertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource) != S_OK)
			return;
		VERTEX_CONSTANT_BUFFER2 *constant_buffer = (VERTEX_CONSTANT_BUFFER2 *)mapped_resource.pData;
		float mvp[4][4] =
		{
			{ 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
			{ 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
			{ 0.0f,         0.0f,           0.5f,       0.0f },
			{ (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
		};

		memcpy(&constant_buffer->mvp, mvp, sizeof(mvp));
		g_pd3dDeviceContext->Unmap(g_pVertexConstantBuffer, 0);

	}
}

void beginFrame(HWND hWnd)
{
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);

	float clear_color[] = { 1.f, 1.f, 1.f, 1.f };
	g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float *)&clear_color);

	setupViewPort(hWnd);
}

void renderFrame()
{
	UINT stride = sizeof(VERTEX_CONSTANT_BUFFER);
	UINT offset = 0;
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));


	if (d3dElems.size() > 0) {
		D3D11_SUBRESOURCE_DATA sr = {};
		for (auto Elem : d3dElems) {

			//Set Buffer Descriptor per elem
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.ByteWidth = sizeof(VERTEX_CONSTANT_BUFFER) * Elem.vertices.size();
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			//Initialize buffer with pointer to vertex data instead of allocating it.
			sr.pSysMem = Elem.vertices.data();

			g_pd3dDevice->CreateBuffer(&bd, &sr, &tempProjectionBuffer);

			if (tempProjectionBuffer) {

				setFontShaderActive();

				g_pd3dDeviceContext->IASetVertexBuffers(0, 1, &tempProjectionBuffer, &stride, &offset);
				g_pd3dDeviceContext->OMSetBlendState(g_pBlendState, NULL, 0xffffffff);

				if (Elem.type == elem_type::text) {
					g_pd3dDeviceContext->PSSetShaderResources(0, 1, &Elem.texture2D);

					//if (Elem->texture2D != nullptr)
						//Elem->texture2D->Release();
				}

				g_pd3dDeviceContext->Draw(Elem.vertices.size(), 0);

				tempProjectionBuffer->Release();
				tempProjectionBuffer = nullptr;

				Elem.vertices.clear();
			}
		}
		d3dElems.clear();
	}
}

void getShaderResourceForChar(unsigned long c, Font *f, ID3D11ShaderResourceView **o, GlyphInfo *glyphInfo)
{
	auto face = f->getFace();
	auto cached = getCachedTextInfoForChar(c, f->getFontSize());

	if (o) {

		if (cached) {
			memcpy(glyphInfo, &cached->glyphInfo, sizeof(GlyphInfo));
			if (cached->shaderResourceView != nullptr) {
				*o = cached->shaderResourceView;
				return;
			}
		}

		FT_Load_Char(face, c, FT_LOAD_RENDER);
		auto slot = face->glyph;

		if (slot->bitmap.width) {
			D3D11_TEXTURE2D_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.Width = slot->bitmap.width;
			desc.Height = slot->bitmap.rows;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;

			ID3D11Texture2D *pTexture = nullptr;

			D3D11_SUBRESOURCE_DATA subResource = {
				slot->bitmap.buffer,
				slot->bitmap.width ,
				0
			};

			//This is using alot of CPU, in order to fix, do not release texture, re-use the same texture for all glyphs.
			g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

			// Create texture view
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			ZeroMemory(&srvDesc, sizeof(srvDesc));
			srvDesc.Format = DXGI_FORMAT_R8_UNORM;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = desc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;

			if (pTexture != nullptr)
			{
				g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, o);

				TexChild *ret = nullptr;
				
				if (!cached) {
					ret = addTextInfoToCache(c, f->getFontSize(), *o, slot->metrics.width, slot->metrics.height, slot->bitmap_left, slot->bitmap_top, slot->advance.x, slot->advance.y);
				}
				else {
					ret = getCachedTextInfoForChar(c, f->getFontSize());
					ret->shaderResourceView = *o;
				}

				memcpy(glyphInfo, &ret->glyphInfo, sizeof(GlyphInfo));
				pTexture->Release();
			}
		}
	}
	else {
		if (!cached) {
			FT_Load_Char(face, c, FT_LOAD_RENDER);
			auto slot = face->glyph;

			TexChild *ret = addTextInfoToCache(c, f->getFontSize(), nullptr, slot->metrics.width, slot->metrics.height, slot->bitmap_left, slot->bitmap_top, slot->advance.x, slot->advance.y);
			memcpy(glyphInfo, &ret->glyphInfo, sizeof(GlyphInfo));
		}
		else {
			memcpy(glyphInfo, &cached->glyphInfo, sizeof(GlyphInfo));
		}
	}
}

/// <summary>
/// Get the width of a rendered string.
/// </summary>
/// <param name="text">is the string to be measured</param>
/// <param name="font">is the desired font of string</param>
/// <param name="fontSize">is the size of the font in pixels.</param>
/// <returns>The width of the string when rendered</returns>
__declspec(dllexport) float __cdecl getTextWidth(const char *text, Font *font, float fontSize)
{
	float w = 0.f;

	font->setFontSize(fontSize);
	for (size_t i = 0; i < strlen(text); i++) {

		GlyphInfo glyphInfo = {};
		getShaderResourceForChar(text[i], font, nullptr, &glyphInfo);

		w += float(glyphInfo.advance_x >> 6);
	}

	return w;
}

__declspec(dllexport) float __cdecl getTextHeight(const char *text, Font *font, float fontSize)
{
	FT_GlyphSlot slot = nullptr;

	GlyphInfo glyphInfo = {};

	font->setFontSize(fontSize);
	getShaderResourceForChar('A', font, nullptr, &glyphInfo);

	float h = float(glyphInfo.height >> 6);

	return h;
}

/// <summary>
/// Draw text onto render target.
/// </summary>
/// <param name="text">is the text to be drawn.</param>
/// <param name="x">the x-coordinate to render the text</param>
/// <param name="y">the y-coordinate to render the text</param>
/// <param name="font">the desired font to render the text as</param>
/// <param name="fontSize">the font size to render the text at</param>
/// <param name="r">the RED channel of the color</param>
/// <param name="g">the GREEN channel of the color</param>
/// <param name="b">the BLUE channel of the color</param>
/// <param name="a">the ALPHA channel of the color</param>
__declspec(dllexport) void __cdecl drawText(const char *text, float x, float y, Font *font, float fontSize, float r, float g, float b, float a)
{
	UINT viewportNumber = 1;

	D3D11_VIEWPORT vp;

	g_pd3dDeviceContext->RSGetViewports(&viewportNumber, &vp);

	XMFLOAT4 color(r / 255.f, g / 255.f, b / 255.f, a / 255.f);

	font->setFontSize(fontSize);

	auto face = font->getFace();

	float currentX = x;
	float currentY = y + fontSize;
	for (size_t i = 0; i < strlen(text); i++) {

		D3DElem elem = {};
		elem.type = elem_type::text;

		ID3D11ShaderResourceView *view = nullptr;

		GlyphInfo glyphInfo = {};

		getShaderResourceForChar(text[i], font, &view, &glyphInfo);

		float x2 = currentX + float(glyphInfo.bitmap_left);
		float y2 = currentY - float(glyphInfo.bitmap_top);

		float w = float(glyphInfo.width >> 6);
		float h = float(glyphInfo.height >> 6);

		elem.texture2D = view;

		VERTEX_CONSTANT_BUFFER vertices[] = {
			{ XMFLOAT2(x2, y2), color, XMFLOAT2(0.f, 0.f) },
			{ XMFLOAT2(x2 + w, y2), color, XMFLOAT2(1.f, 0.f)  },
			{ XMFLOAT2(x2, y2 + h), color, XMFLOAT2(0.f, 1.f) },
			{ XMFLOAT2(x2 + w, y2 + h), color, XMFLOAT2(1.f, 1.f) }
		};

		for (int i = 0; i < 4; i++) {
			elem.addVertex(vertices[i]);
		}

		d3dElems.push_back(elem);

		currentX += glyphInfo.advance_x >> 6;
		currentY += glyphInfo.advance_y >> 6;
	}
}

/// <summary>
/// Render a rectangle without any texture.
/// </summary>
/// <param name="x">The x-coordinate to render the recangle</param>
/// <param name="y">The y-coordinate to render the rectangle</param>
/// <param name="w">The width of the rectangle to render</param>
/// <param name="h">The height of the rectangle to render</param>
/// <param name="r">The RED channel of the rectangle's color</param>
/// <param name="g">The GREEN channel of the rectangle's color</param>
/// <param name="b">The BLUE channel of the rectangle's color</param>
/// <param name="a">The ALPHA channel of the rectangle's color</param>
__declspec(dllexport) void __cdecl drawBox(float x, float y, float w, float h, float r, float g, float b, float a)
{
	UINT viewportNumber = 1;

	D3D11_VIEWPORT vp;

	g_pd3dDeviceContext->RSGetViewports(&viewportNumber, &vp);

	XMFLOAT4 color(r / 255.f, g / 255.f, b / 255.f, a / 255.f);

	VERTEX_CONSTANT_BUFFER vertices[] = {
	{ XMFLOAT2(x, y), color, XMFLOAT2(-1.f, -1.f) },
	{ XMFLOAT2(x + w, y), color, XMFLOAT2(-1.f, -1.f)  },
	{ XMFLOAT2(x, y + h), color, XMFLOAT2(-1.f, -1.f) },
	{ XMFLOAT2(x + w, y + h), color, XMFLOAT2(-1.f, -1.f) }
	};


	D3DElem elem = {};
	elem.type = elem_type::hud;
	elem.texture2D = nullptr;


	for (int i = 0; i < 4; i++) {
		elem.addVertex(vertices[i]);
	}

	d3dElems.push_back(elem);
}

__declspec(dllexport) Font *__cdecl getRobotoFont()
{
	return roboto;
}

__declspec(dllexport) HWND __cdecl getWindowHWND()
{
	return g_hWnd;
}

void CreateFontTex()
{
	Font::initFreeType();

	roboto = new Font("C:\\Users\\Paul\\Source\\repos\\ConsoleApplication1\\Release\\roboto.ttf");
}

/// <summary>
/// Initialize a window as well as D3D11.
/// </summary>
/// <param name="WndProc2">A pointer to a function for custom message processing</param>
/// <param name="DrawingFuntion">A pointer to a function which will call the exports drawBox() and drawText().</param>
/// <example> A small example on how to use this library.
/// <code>
/// __declspec(dllimport) void __cdecl initializeWindow(WNDPROC WndProc2, void(*DrawingFuntion)());
/// __declspec(dllimport) void __cdecl drawBox(float x, float y, float w, float h, float r, float g, float b, float a);
/// __declspec(dllimport) void __cdecl drawText(const char *text, float x, float y, class Font *font, float fontSize, float r, float g, float b, float a);
/// __declspec(dllexport) class Font *__cdecl getRobotoFont();
/// __declspec(dllimport) float __cdecl getTextWidth(const char *text, class Font *font, float fontSize);
/// __declspec(dllimport) float __cdecl getTextHeight(const char *text, Font *font, float fontSize);
/// __declspec(dllimport) HWND __cdecl getWindowHWND();
/// 
/// void Drawing()
/// {
///		//This will draw the text "Hello World!" with a line under-lining it.
///		drawText("Hello World!", 100, 100, getRobotoFont(), 16.f, 0, 0, 0, 255);
///		drawBox(100, 116, getTextWidth("Hello World!", getRobotoFont(), 16.f), 2, 0, 0, 0, 255);
/// }
/// 
/// int main()
/// {
///		initializeWindow(nullptr, Drawing);
///		return 0;
/// }
/// </code>
/// </example>
/// <returns></returns>
__declspec(dllexport) void __cdecl initializeWindow(WNDPROC WndProc2, void(*DrawingFuntion)())
{
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("DirectX 11"), NULL };
	::RegisterClassEx(&wc);
	HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("DirectX 11"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);
	g_hWnd = hwnd;

	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return;
	}

	// Show the window
	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);

	ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	if (InitializeFontShader() == false)
		printf("InitializeFontShader(): Failed.\n");

	{
		D3D11_BUFFER_DESC desc;
		desc.ByteWidth = sizeof(VERTEX_CONSTANT_BUFFER2);
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;

		g_pd3dDevice->CreateBuffer(&desc, nullptr, &g_pVertexConstantBuffer);
	}

	{
		D3D11_BLEND_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.AlphaToCoverageEnable = false;
		desc.RenderTarget[0].BlendEnable = true;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		g_pd3dDevice->CreateBlendState(&desc, &g_pBlendState);
	}

	// Create the rasterizer state
	{
		D3D11_RASTERIZER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_NONE;
		desc.ScissorEnable = true;
		desc.DepthClipEnable = true;
		g_pd3dDevice->CreateRasterizerState(&desc, &g_pRasterizerState);
	}

	// Create depth-stencil State
	{
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.DepthEnable = false;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		desc.StencilEnable = false;
		desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.BackFace = desc.FrontFace;
		g_pd3dDevice->CreateDepthStencilState(&desc, &g_pDepthStencilState);
	}

	CreateFontTex();

	if (WndProc2) {
		WndProc2_Client = WndProc2;
	}

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			continue;
		}

		beginFrame(hwnd);

		if (DrawingFuntion) {
			DrawingFuntion();
		}

		renderFrame();

		g_pSwapChain->Present(1, 0);
	}

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


#pragma once

#include <Windows.h>
#include <windowsx.h>
#include <iostream>
#include <stdio.h>
#include <tchar.h>

#include <stdint.h>
#include <vector>

#include <d3d11.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
using namespace DirectX;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment (lib, "D3D11.lib")



#include <ft2build.h>
#include <freetype2/freetype/freetype.h>
#include <freetype2/freetype/ftglyph.h>

struct VERTEX_CONSTANT_BUFFER;
struct VERTEX_CONSTANT_BUFFER2;

#include "D3DElem.h"
#include "FontShader.h"
#include "Font.h"
#include "FontTexture.h"

struct VERTEX_CONSTANT_BUFFER
{
	XMFLOAT2 pos;
	XMFLOAT4 col;
	XMFLOAT2 uv;
};

struct VERTEX_CONSTANT_BUFFER2
{
	float   mvp[4][4];
};

extern ID3D11Device *g_pd3dDevice;
extern ID3D11DeviceContext *g_pd3dDeviceContext;
extern IDXGISwapChain *g_pSwapChain;
extern ID3D11RenderTargetView *g_mainRenderTargetView;
extern IDXGIFactory *g_pFactory;

extern ID3D11Buffer *g_pVertexConstantBuffer;






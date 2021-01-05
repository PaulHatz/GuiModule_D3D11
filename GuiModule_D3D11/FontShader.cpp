#include "main.h"

ID3D11VertexShader *fontVertexShader = nullptr;
ID3D11PixelShader *fontPixelShader = nullptr;
ID3D11InputLayout *fontTextureInputLayout = nullptr;
ID3D11SamplerState *fontTextureSampler = nullptr;


const char *vertexShader =
"cbuffer vertexBuffer : register(b0) \
            {\
              float4x4 ProjectionMatrix; \
            };\
            struct VS_INPUT\
            {\
              float2 pos : POSITION;\
              float4 col : COLOR0;\
              float2 uv  : TEXCOORD0;\
            };\
            \
            struct PS_INPUT\
            {\
              float4 pos : SV_POSITION;\
              float4 col : COLOR0;\
              float2 uv  : TEXCOORD0;\
            };\
            \
            PS_INPUT main(VS_INPUT input)\
            {\
              PS_INPUT output;\
              output.pos = mul(ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\
              output.col = input.col;\
              output.uv  = input.uv;\
              return output;\
            }";

const char *pixelShader =
"struct PS_INPUT\
            {\
				float4 pos : SV_POSITION;\
				float4 col : COLOR0;\
				float2 uv  : TEXCOORD0;\
            };\
			sampler sampler0; \
			Texture2D texture0;\
            float4 main(PS_INPUT input) : SV_Target\
            {\
				if(input.uv.x == -1) return input.col;\
				float4 uv = texture0.Sample(sampler0, input.uv);\
				return float4(input.col.xyz, input.col.w * uv.x); \
            }";


bool InitializeFontShader()
{
	ID3DBlob *vsBlob = nullptr;
	ID3DBlob *psBlob = nullptr;

	D3DCompile(vertexShader, strlen(vertexShader), nullptr, nullptr, nullptr, "main", "vs_4_0", 0, 0, &vsBlob, nullptr);
	if (g_pd3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &fontVertexShader) == S_OK)
	{
		D3D11_INPUT_ELEMENT_DESC local_layout[] =
		{
				{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, 24,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		if (g_pd3dDevice->CreateInputLayout(local_layout, 3, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &fontTextureInputLayout) == S_OK)
		{
			vsBlob->Release();
			
			D3DCompile(pixelShader, strlen(pixelShader), nullptr, nullptr, nullptr, "main", "ps_4_0", 0, 0, &psBlob, nullptr);
			if (g_pd3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &fontPixelShader) != S_OK)
			{
				D3D11_SAMPLER_DESC desc;
				ZeroMemory(&desc, sizeof(desc));
				desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
				desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
				desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
				desc.MipLODBias = 0.f;
				desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
				desc.MinLOD = 0.f;
				desc.MaxLOD = 0.f;
				g_pd3dDevice->CreateSamplerState(&desc, &fontTextureSampler);

				psBlob->Release();
				return true;
			}
			psBlob->Release();
			return false;
		}
		vsBlob->Release();
		return false;
	}


	vsBlob->Release();
	return false;
}

void setFontShaderActive()
{
	g_pd3dDeviceContext->VSSetShader(fontVertexShader, NULL, 0);
	g_pd3dDeviceContext->VSSetConstantBuffers(0, 1, &g_pVertexConstantBuffer);
	g_pd3dDeviceContext->PSSetShader(fontPixelShader, NULL, 0);
	g_pd3dDeviceContext->PSSetSamplers(0, 1, &fontTextureSampler);

	g_pd3dDeviceContext->IASetInputLayout(fontTextureInputLayout);
	g_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}
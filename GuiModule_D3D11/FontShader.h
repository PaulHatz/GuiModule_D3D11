#pragma once
extern ID3D11VertexShader *fontVertexShader;
extern ID3D11PixelShader *fontPixelShader;
extern ID3D11InputLayout *fontTextureInputLayout;
extern ID3D11SamplerState *fontTextureSampler;

extern bool InitializeFontShader();
extern void setFontShaderActive();
#pragma once
enum class elem_type
{
	none,
	hud,
	text
};

struct D3DElem
{
	elem_type type;
	std::vector<VERTEX_CONSTANT_BUFFER> vertices;
	ID3D11ShaderResourceView *texture2D;

	void addVertex(VERTEX_CONSTANT_BUFFER vertex);

	D3DElem();
	~D3DElem();
};
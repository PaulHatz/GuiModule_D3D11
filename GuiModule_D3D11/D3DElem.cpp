#include "main.h"

void D3DElem::addVertex(VERTEX_CONSTANT_BUFFER vertex)
{
	vertices.push_back(vertex);
}

D3DElem::D3DElem()
{
	this->type = elem_type::none;
	this->texture2D = nullptr;
}

D3DElem::~D3DElem()
{
	this->vertices.clear();
	this->texture2D = nullptr;
}
// Copyright (c) Peter Hillerström (skipifzero.com, peter@hstroem.se)

#include "sfz/gl/Model.hpp"

#include <algorithm>
#include <cstddef> // offsetof()
#include <string>
#include <vector>

#include "tiny_obj_loader.h"

#include "sfz/Assert.hpp"
#include "sfz/gl/IncludeOpenGL.hpp"

namespace sfz {

namespace gl {

using std::string;
using std::vector;

using tinyobj::shape_t;
using tinyobj::material_t;

// Model class
// ------------------------------------------------------------------------------------------------

Model::Model(Model&& other) noexcept
{
	this->swap(other);
}

Model& Model::operator= (Model&& other) noexcept
{
	this->swap(other);
	return *this;
}

Model::~Model() noexcept
{
	this->destroy();
}

// Model class
// ------------------------------------------------------------------------------------------------

void Model::swap(Model& other) noexcept
{
	this->vertices.swap(other.vertices);
	this->indices.swap(other.indices);

	std::swap(this->glVertexBuffer, other.glVertexBuffer);
	std::swap(this->glIndexBuffer, other.glIndexBuffer);
	std::swap(this->glVAO, other.glVAO);

	std::swap(this->glColorTexture, other.glColorTexture);
}

void Model::destroy() noexcept
{
	this->vertices.destroy();
	this->indices.destroy();

	// Silently ignores values == 0
	glDeleteBuffers(1, &glVertexBuffer);
	glDeleteBuffers(1, &glIndexBuffer);
	glDeleteVertexArrays(1, &glVAO);

	glDeleteTextures(1, &glColorTexture);
}

void Model::draw() const noexcept
{
	glBindVertexArray(glVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glIndexBuffer);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

// Model loading functions
// ------------------------------------------------------------------------------------------------

Model tinyObjLoadModel(const char* basePath, const char* fileName) noexcept
{
	vector<shape_t> shapes;
	vector<material_t> materials;

	string error = tinyobj::LoadObj(shapes, materials, (string(basePath) + fileName).c_str(), basePath);
	if (!error.empty()) {
		printErrorMessage("Failed loading model %s, error: %s", fileName, error.c_str());
		return Model();
	}

	// TODO: Only cares about first shape
	if (shapes.size() == 0) {
		printErrorMessage("Model %s has no shapes", fileName);
		return Model();
	}
	shape_t& shape = shapes[0];

	// Calculate number of vertices and create default vertices (all 0)
	size_t numVertices = std::max(shape.mesh.positions.size() / 3,
	                     std::max(shape.mesh.normals.size() / 3, shape.mesh.texcoords.size() / 2));
	Model tmp;
	tmp.vertices = DynArray<Vertex>(numVertices, numVertices);
	
	// Fill vertices with positions
	for (size_t i = 0; i < shape.mesh.positions.size() / 3; i++) {
		tmp.vertices[i].pos = vec3(&shape.mesh.positions[i * 3]);
	}

	// Fill vertices with normals
	for (size_t i = 0; i < shape.mesh.normals.size() / 3; i++) {
		tmp.vertices[i].normal = vec3(&shape.mesh.normals[i * 3]);
	}

	// Fill vertices with uv coordinates
	for (size_t i = 0; i < shape.mesh.texcoords.size() / 2; i++) {
		tmp.vertices[i].uv = vec2(&shape.mesh.texcoords[i * 2]);
	}

	// Create indices
	tmp.indices.add(&shape.mesh.indices[0], shape.mesh.indices.size());

	// Create Vertex Array object
	glGenVertexArrays(1, &tmp.glVAO);
	glBindVertexArray(tmp.glVAO);

	// Create and fill vertex buffer
	glGenBuffers(1, &tmp.glVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, tmp.glVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, tmp.vertices.size() * sizeof(Vertex), tmp.vertices.data(), GL_STATIC_DRAW);

	// Locate components in vertex buffer
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	// Create and fill index buffer
	glGenBuffers(1, &tmp.glIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tmp.glIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * tmp.indices.size(), tmp.indices.data(), GL_STATIC_DRAW);

	// Cleanup
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return std::move(tmp);
}

} // namespace gl
} // namespace sfz

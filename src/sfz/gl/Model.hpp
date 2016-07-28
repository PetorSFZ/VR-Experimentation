// Copyright (c) Peter Hillerström (skipifzero.com, peter@hstroem.se)

#pragma once

#include <cstdint>

#include "sfz/containers/DynArray.hpp"
#include "sfz/math/Matrix.hpp"
#include "sfz/math/Vector.hpp"

namespace sfz {

namespace gl {

using std::uint32_t;

// Vertex struct
// ------------------------------------------------------------------------------------------------

struct Vertex {
	vec3 pos = vec3(0.0);
	vec3 normal = vec3(0.0);
	vec2 uv = vec2(0.0);
};

static_assert(sizeof(Vertex) == sizeof(float) * 8, "Vertex is padded");

// Model class
// ------------------------------------------------------------------------------------------------

/// A class holding a 3d model
/// All information is available through public members, it is however very important that you do
/// not modify these members unless you know exactly what you are doing. The reason for them being
/// public in the first place is to make it easier to do special modifications when necessary.
class Model {
public:
	// Public members
	// --------------------------------------------------------------------------------------------

	// Raw geometric information
	DynArray<Vertex> vertices;
	DynArray<uint32_t> indices;

	// OpenGL geometric information
	uint32_t glVertexBuffer = 0;
	uint32_t glIndexBuffer = 0;
	uint32_t glVAO = 0;

	// OpenGL textures
	uint32_t glColorTexture = 0;

	// Constructors & destructors
	// --------------------------------------------------------------------------------------------

	Model() noexcept = default;
	Model(const Model&) = delete;
	Model& operator= (const Model&) = delete;
	
	Model(Model&& other) noexcept;
	Model& operator= (Model&& other) noexcept;
	~Model() noexcept;

	// Public methods
	// --------------------------------------------------------------------------------------------

	/// Swaps this model with another model
	void swap(Model& other) noexcept;

	/// Destroys this model, the result will be an empty Model.
	void destroy() noexcept;

	/// Draws the geometry of this model through OpenGL, material information (including binding
	/// textures) needs to be done manually before the call.
	void draw() const noexcept;
};

// Model loading functions
// ------------------------------------------------------------------------------------------------

/// Loads a 3D model through tinyObjLoader, returns an empty Model on failure.
Model tinyObjLoadModel(const char* basePath, const char* fileName) noexcept;

} // namespace gl
} // namespace sfz

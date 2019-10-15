#pragma once
#include "VertexBuffer.h"
#include "Color.h"
// This class should allow the ability to draw a HUGE # of dynamically updated
//	meshes that have identical geometry.
// This uses modern OpenGL (4.x) vertex specification techniques, as well as
//	drawing techniques, outlined in these wiki pages:
//	https://www.khronos.org/opengl/wiki/Vertex_Specification#Separate_attribute_format
//	https://www.khronos.org/opengl/wiki/Vertex_Rendering#Instancing
// Drawing is performed using OpenGL 4.2 feature 'glDrawArraysInstancedBaseInstance'
// A mesh cache does not care about draw order (either at the instance orders, 
//	or the mesh batch orders), so the caller is responsible for ordering the 
//	instance model data & the mesh data.
///TODO: rename this to 'DynamicInstancedMeshCache' since there is the 
///	implication that the instances of these meshes are very likely to change
///	between every frame?  Or perhaps just manage two types of MeshData: 
///	dynamic & static mesh instances.
class InstancedMeshCache
{
public:
	using MeshId = u16;
	static const MeshId INVALID_MESH_ID = 
		std::numeric_limits<MeshId>::max();
public:
	// max mesh vertex count represents the max total sum of vertices of all 
	//	meshes created using 'addMesh'.  For a 2D game, this probably should
	//	be around 3 figures.
	// max total instances represents the maximum total calls to 'batchModel'
	//	per frame.  This is what we're trying to maximize, and the target #
	//	is a low 7 figures (about 1 million)
	// max batch count represents the upper-bound for the # of draw calls we
	//	want to dispatch to the GPU per frame.  The larger this # is, the 
	//	worse performance we will get.  Try to keep this as close as possible
	//	to the sum of all primitive batches for all meshes added via 'addMesh'
	//	(probably sub 3 figures)
	bool create(size_t maxTotalMeshVertexCount,
				size_t maxTotalInstances,
				size_t maxBatchCount);
	void destroy();
	MeshId addMesh(size_t maxInstances,
				   vector<v2f> const& vertexPositions, 
				   vector<Color> const& vertexColors,
				   vector<GLenum> const& primitiveTypes,
				   vector<GLint> const& primitiveVertexCounts);
	void updateMeshInstances(MeshId mid, size_t instanceCount,
							 v2f const* instanceModelTranslations, 
							 float const* instanceModelRadians,
							 v2f const* instanceModelScales);
	// draw performs the following tasks:
	//	-sets up the gfx pipeline to use the correct VAO(s)
	//	-submits the OpenGL draw calls
	void draw(VertexArray const& vaTextureless);
private:
	size_t maxTotalMeshVertexCount;
	size_t maxTotalInstances;
	size_t maxBatchCount;
	struct PrimitiveBatch
	{
		GLenum primitiveType;
		GLint meshVertexStartIndex;
		GLsizei meshVertexCount;
	};
	struct MeshData
	{
		vector<PrimitiveBatch> primitiveBatches;
		GLuint baseInstance;
		GLsizei instanceCount;
		GLsizei instanceCountMax;
	};
	// There is no maximum # of meshes allowed because it is assumed that the
	//	user is going to build a relatively small # of meshes very few times
	//	during execution, and these meshes are going to be referred to many
	//	times via batches.  Therefore we can assume minimal impact of 
	//	dynamically allocating more space for additional mesh data as needed.
	vector<MeshData> meshData;
	// static mesh per-vertex data buffers //
	VertexBuffer vbPosition;
	VertexBuffer vbColor;
	// super-dynamic instance buffers that need to be updated once per-frame //
	VertexBuffer vbInstanceModelTranslation;
	VertexBuffer vbInstanceModelRadians;
	VertexBuffer vbInstanceModelScale;
};

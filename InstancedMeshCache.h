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
///TODO: rename this class to InstancedMeshBatcher
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
	MeshId addMesh(vector<v2f> const& vertexPositions, 
				   vector<Color> const& vertexColors,
				   vector<GLenum> const& primitiveTypes,
				   vector<GLint> const& primitiveVertexCounts);
	///TODO: make this thread-safe
	void batchModel(MeshId mid, v2f const& translation, float radians,
					v2f const& scale);
	// draw performs the following tasks:
	//	-sets up the gfx pipeline to use the correct VAO(s)
	//	-sends the contents of the instance buffers => GPU
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
	};
	// There is no maximum # of meshes allowed because it is assumed that the
	//	user is going to build a relatively small # of meshes very few times
	//	during execution, and these meshes are going to be referred to many
	//	times via batches.  Therefore we can assume minimal impact of 
	//	dynamically allocating more space for additional mesh data as needed.
	vector<MeshData> meshData;
	struct MeshBatch
	{
		size_t meshIndex;
		GLuint baseInstance;
		GLsizei instanceCount;
	};
	vector<MeshBatch> batches;
	// initialized to -1 at the beginning of the first frame & after ever call 
	//	to 'draw'.  This value directs calls to 'batchModel' which part of the
	//	'batches' array it should operate on.
	int currentBatchIndex;
	// static mesh per-vertex data buffers //
	VertexBuffer vbPosition;
	VertexBuffer vbColor;
	// super-dynamic instance buffers that need to be updated once per-frame //
	VertexBuffer vbInstanceModelTranslation;
	VertexBuffer vbInstanceModelRadians;
	VertexBuffer vbInstanceModelScale;
	// CPU data buffers that will get sent to the GPU //
	vector<v2f>   instanceModelTranslations;
	vector<float> instanceModelRadians;
	vector<v2f>   instanceModelScale;
};

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
class InstancedMeshCache
{
public:
	using MeshId     = u16;
	using InstanceId = u32;
	static const MeshId INVALID_MESH_ID = 
		std::numeric_limits<MeshId>::max();
	static const InstanceId INVALID_INSTANCE_ID = 
		std::numeric_limits<InstanceId>::max();
public:
	bool create(size_t maxTotalMeshVertexCount,
				InstanceId maxTotalInstances);
	void destroy();
	MeshId addMesh(vector<v2f> const& vertexPositions, 
				   vector<Color> const& vertexColors,
				   InstanceId maxInstanceCount,
				   vector<GLenum> const& primitiveTypes,
				   vector<GLint> const& primitiveVertexCounts);
	InstanceId createInstance(MeshId mid);
	void destroyInstance(MeshId mid, InstanceId iid);
	void setModel(InstanceId iid, v2f const& translation, float radians,
				  v2f const* scale);
	// this function does two things:
	//	1) sends the contents of the instance buffers => GPU
	//	2) submits the OpenGL draw calls
	void draw(VertexArray const& vaTextureless);
private:
	size_t maxTotalMeshVertexCount;
	InstanceId maxTotalInstances;
	struct PrimitiveBatch
	{
		GLenum primitiveType;
		GLint modelVertexStartIndex;
		GLsizei modelVertexCount;
	};
	struct MeshData
	{
		// VertexBuffer meta data //
		vector<PrimitiveBatch> primitiveBatches;
		InstanceId instanceCount;
		GLuint baseInstance;
		// instance lookup //
		vector<InstanceId> availableInstanceIds;
		size_t nextAvailableInstanceIdIndex;
	};
	vector<MeshData> meshData;
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
	bool instanceScalesDirty;
	vector<v2f>   instanceModelScale;
};

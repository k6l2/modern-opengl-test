#include "InstancedMeshCache.h"
bool InstancedMeshCache::create(size_t maxTotalMeshVertexCount,
								InstanceId maxTotalInstances)
{
	this->maxTotalMeshVertexCount = maxTotalMeshVertexCount;
	this->maxTotalInstances = maxTotalInstances;
	if (!vbPosition.create(maxTotalMeshVertexCount, sizeof(v2f), 
						   VertexBuffer::MemoryUsage::STATIC))
	{
		SDL_assert(false);
		return false;
	}
	if (!vbColor.create(maxTotalMeshVertexCount, sizeof(Color),
						VertexBuffer::MemoryUsage::STATIC))
	{
		SDL_assert(false);
		return false;
	}
	if (!vbInstanceModelTranslation.create(maxTotalInstances, sizeof(v2f),
										   VertexBuffer::MemoryUsage::STREAM))
	{
		SDL_assert(false);
		return false;
	}
	if (!vbInstanceModelRadians.create(maxTotalInstances, sizeof(float),
									   VertexBuffer::MemoryUsage::STREAM))
	{
		SDL_assert(false);
		return false;
	}
	if (!vbInstanceModelScale.create(maxTotalInstances, sizeof(v2f),
									 VertexBuffer::MemoryUsage::DYNAMIC))
	{
		SDL_assert(false);
		return false;
	}
	instanceModelTranslations.resize(maxTotalInstances);
	instanceModelRadians.resize(maxTotalInstances);
	instanceModelScale.resize(maxTotalInstances, v2f(0,0));
	instanceScalesDirty = true;
	return true;
}
void InstancedMeshCache::destroy()
{
	vbPosition.destroy();
	vbColor.destroy();
	vbInstanceModelTranslation.destroy();
	vbInstanceModelRadians.destroy();
}
InstancedMeshCache::MeshId 
	InstancedMeshCache::addMesh(vector<v2f> const& vertexPositions,
								vector<Color> const& vertexColors,
								InstanceId maxInstanceCount,
								GLenum primitiveType)
{
	SDL_assert(vertexPositions.size() == vertexColors.size());
	SDL_assert(maxInstanceCount > 0);
	const GLuint baseInstance = meshData.empty() ? 0 :
		meshData.back().baseInstance + 
			meshData.back().instanceCount;
	vector<InstanceId> availableInstanceIds(maxInstanceCount);
	for (size_t iid = 0; iid < maxInstanceCount; iid++)
	{
		availableInstanceIds[iid] = static_cast<InstanceId>(baseInstance + iid);
	}
	const MeshData newMeshDat
	{
		primitiveType,
		meshData.empty() ? 0 : 
			meshData.back().modelVertexStartIndex +
				meshData.back().modelVertexCount,// modelVertexStartIndex
		static_cast<GLsizei>(vertexPositions.size()),// modelVertexCount
		maxInstanceCount,// instanceCount
		baseInstance,// baseInstance
		availableInstanceIds,// availableInstanceIds
		0// nextAvailableInstanceIdIndex
	};
	if (newMeshDat.modelVertexStartIndex + vertexPositions.size() >
			maxTotalMeshVertexCount)
	{
		SDL_assert(false);
		return INVALID_MESH_ID;
	}
	vbPosition.update(vertexPositions.data(), newMeshDat.modelVertexStartIndex,
					  vertexPositions.size());
	vbColor.update(vertexColors.data(), newMeshDat.modelVertexStartIndex,
				   vertexColors.size());
	SDL_assert(meshData.size() < INVALID_MESH_ID);
	MeshId retVal = static_cast<MeshId>(meshData.size());
	meshData.push_back(newMeshDat);
	return retVal;
}
InstancedMeshCache::InstanceId InstancedMeshCache::createInstance(MeshId mid)
{
	SDL_assert(mid < meshData.size());
	MeshData& mDat = meshData[mid];
	if (mDat.nextAvailableInstanceIdIndex >= mDat.availableInstanceIds.size())
	{
		// we cannot create any more instances of this mesh..
		SDL_assert(false);
		return INVALID_INSTANCE_ID;
	}
	const InstanceId retVal =
		mDat.availableInstanceIds[mDat.nextAvailableInstanceIdIndex++];
	// by default, initialize the new instance to have a normal scale so we 
	//	don't accidentally forget to do this and not know why it isn't showing
	//	up.
	instanceModelScale[retVal] = v2f(1,1);
	instanceScalesDirty = true;
	return retVal;
}
void InstancedMeshCache::destroyInstance(MeshId mid, InstanceId iid)
{
	SDL_assert(mid < meshData.size());
	SDL_assert(iid < instanceModelScale.size());
	instanceModelScale[iid] = v2f(0, 0);
	instanceScalesDirty = true;
	MeshData& mDat = meshData[mid];
#ifndef NDEBUG
	// Check to make sure that iid has not already been destroyed.  If we have
	//	a LOT of instances, this is probably expensive to do so don't do it in
	//	production?..
	auto iidIt = std::find(mDat.availableInstanceIds.begin() + 
						   	mDat.nextAvailableInstanceIdIndex,
						   mDat.availableInstanceIds.end(), iid);
	SDL_assert(iidIt == mDat.availableInstanceIds.end());
#endif
	SDL_assert(mDat.nextAvailableInstanceIdIndex > 0);
	mDat.availableInstanceIds[--mDat.nextAvailableInstanceIdIndex] = iid;
}
void InstancedMeshCache::setModel(InstanceId iid, 
								  v2f const& translation, float radians, 
								  v2f const* scale)
{
	instanceModelTranslations[iid] = translation;
	instanceModelRadians     [iid] = radians;
	if (scale)
	{
		instanceModelScale[iid] = *scale;
		instanceScalesDirty = true;
	}
}
void InstancedMeshCache::draw(VertexArray const& vaTextureless)
{
	vbInstanceModelTranslation.update(instanceModelTranslations.data());
	vbInstanceModelRadians    .update(instanceModelRadians.data());
	if (instanceScalesDirty)
	{
		vbInstanceModelScale.update(instanceModelScale.data());
		instanceScalesDirty = false;
	}
	VertexArray::use(&vaTextureless, k10::gGlobalUniformBuffer,
					 { &vbPosition, &vbColor, &vbInstanceModelTranslation, 
					 	&vbInstanceModelRadians, &vbInstanceModelScale} );
	for (MeshData const& meshDat : meshData)
	{
		glDrawArraysInstancedBaseInstance(meshDat.primitiveType,
			meshDat.modelVertexStartIndex, meshDat.modelVertexCount,
			meshDat.instanceCount, meshDat.baseInstance);
	}
}
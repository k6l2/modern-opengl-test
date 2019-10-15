#include "InstancedMeshCache.h"
bool InstancedMeshCache::create(size_t maxTotalMeshVertexCount,
								size_t maxTotalInstances,
								size_t maxBatchCount)
{
	this->maxTotalMeshVertexCount = maxTotalMeshVertexCount;
	this->maxTotalInstances       = maxTotalInstances;
	this->maxBatchCount           = maxBatchCount;
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
									 VertexBuffer::MemoryUsage::STREAM))
	{
		SDL_assert(false);
		return false;
	}
	instanceModelTranslations.resize(maxTotalInstances);
	instanceModelRadians.resize(maxTotalInstances);
	instanceModelScale.resize(maxTotalInstances, v2f(0,0));
	batches.resize(maxBatchCount);
	currentBatchIndex = -1;
	return true;
}
void InstancedMeshCache::destroy()
{
	vbPosition.destroy();
	vbColor.destroy();
	vbInstanceModelTranslation.destroy();
	vbInstanceModelRadians.destroy();
	vbInstanceModelScale.destroy();
}
InstancedMeshCache::MeshId 
	InstancedMeshCache::addMesh(vector<v2f> const& vertexPositions,
								vector<Color> const& vertexColors,
								vector<GLenum> const& primitiveTypes,
								vector<GLint> const& primitiveVertexCounts)
{
	SDL_assert(vertexPositions.size() == vertexColors.size());
	SDL_assert(!primitiveTypes.empty());
	SDL_assert(primitiveTypes.size() == primitiveVertexCounts.size());
///	const GLuint baseInstance = meshData.empty() ? 0 :
///		meshData.back().baseInstance + 
///			meshData.back().instanceCount;
///	vector<InstanceId> availableInstanceIds(maxInstanceCount);
///	for (size_t iid = 0; iid < maxInstanceCount; iid++)
///	{
///		availableInstanceIds[iid] = static_cast<InstanceId>(baseInstance + iid);
///	}
	vector<PrimitiveBatch> primitiveBatches(primitiveTypes.size());
	GLint currModelVertexStartIndex = meshData.empty() ? 0 :
		meshData.back().primitiveBatches.back().meshVertexStartIndex +
			meshData.back().primitiveBatches.back().meshVertexCount;
	for (size_t p = 0; p < primitiveTypes.size(); p++)
	{
		const PrimitiveBatch pBatch = {
			primitiveTypes[p],
			currModelVertexStartIndex,
			primitiveVertexCounts[p]
		};
		primitiveBatches[p] = pBatch;
		currModelVertexStartIndex += primitiveVertexCounts[p];
	}
	const MeshData newMeshDat
	{
		primitiveBatches,
//////		meshData.empty() ? 0 : 
//////			meshData.back().modelVertexStartIndex +
//////				meshData.back().modelVertexCount,// modelVertexStartIndex
//////		static_cast<GLsizei>(vertexPositions.size()),// modelVertexCount
///		maxInstanceCount,// instanceCount
///		baseInstance,// baseInstance
///		availableInstanceIds,// availableInstanceIds
///		0// nextAvailableInstanceIdIndex
	};
	if (primitiveBatches.front().meshVertexStartIndex + 
				vertexPositions.size() >
			maxTotalMeshVertexCount)
	{
		SDL_assert(false);
		return INVALID_MESH_ID;
	}
	vbPosition.update(vertexPositions.data(), 
					  primitiveBatches.front().meshVertexStartIndex,
					  vertexPositions.size());
	vbColor.update(vertexColors.data(),
				   primitiveBatches.front().meshVertexStartIndex,
				   vertexColors.size());
	SDL_assert(meshData.size() < INVALID_MESH_ID);
	MeshId retVal = static_cast<MeshId>(meshData.size());
	meshData.push_back(newMeshDat);
	return retVal;
}
///InstancedMeshCache::InstanceId InstancedMeshCache::createInstance(MeshId mid)
///{
///	SDL_assert(mid < meshData.size());
///	MeshData& mDat = meshData[mid];
///	if (mDat.nextAvailableInstanceIdIndex >= mDat.availableInstanceIds.size())
///	{
///		// we cannot create any more instances of this mesh..
///		SDL_assert(false);
///		return INVALID_INSTANCE_ID;
///	}
///	const InstanceId retVal =
///		mDat.availableInstanceIds[mDat.nextAvailableInstanceIdIndex++];
///	// by default, initialize the new instance to have a normal scale so we 
///	//	don't accidentally forget to do this and not know why it isn't showing
///	//	up.
///	instanceModelScale[retVal] = v2f(1,1);
///	//vbInstanceModelScale.update(&instanceModelScale[retVal], retVal, 1);
//////	instanceScalesDirty = true;
///	return retVal;
///}
///void InstancedMeshCache::destroyInstance(MeshId mid, InstanceId iid)
///{
///	SDL_assert(mid < meshData.size());
///	SDL_assert(iid < instanceModelScale.size());
///	instanceModelScale[iid] = v2f(0, 0);
///	//vbInstanceModelScale.update(&instanceModelScale[iid], iid, 1);
//////	instanceScalesDirty = true;
///	MeshData& mDat = meshData[mid];
///#ifndef NDEBUG
///	// Check to make sure that iid has not already been destroyed.  If we have
///	//	a LOT of instances, this is probably expensive to do so don't do it in
///	//	production?..
///	auto iidIt = std::find(mDat.availableInstanceIds.begin() + 
///						   	mDat.nextAvailableInstanceIdIndex,
///						   mDat.availableInstanceIds.end(), iid);
///	SDL_assert(iidIt == mDat.availableInstanceIds.end());
///#endif
///	SDL_assert(mDat.nextAvailableInstanceIdIndex > 0);
///	mDat.availableInstanceIds[--mDat.nextAvailableInstanceIdIndex] = iid;
///}
void InstancedMeshCache::batchModel(MeshId mid, 
									v2f const& translation, float radians, 
									v2f const& scale)
{
	// Within the commented 'CRITICAL' sections outlined in this function, we
	//	need to guarantee that the state of the critical data is kept safe from
	//	other threads if we want this function to be thread-safe.
	// BEGIN CRITICAL currentBatchIndex //
	if (currentBatchIndex < 0 || batches[currentBatchIndex].meshIndex != mid)
	{
		if (currentBatchIndex >= 0 && currentBatchIndex >= batches.size() - 1)
		{
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
				"Frame batch limit exceeded!\n");
			SDL_assert(false);
			return;
		}
		currentBatchIndex++;
		batches[currentBatchIndex].meshIndex = mid;
		batches[currentBatchIndex].instanceCount = 0;
		if (currentBatchIndex > 0)
		{
			batches[currentBatchIndex].baseInstance =
				batches[currentBatchIndex - 1].baseInstance + 
					batches[currentBatchIndex - 1].instanceCount;
		}
		else
		{
			batches[currentBatchIndex].baseInstance = 0;
		}
	}
	MeshBatch& meshBatch = batches[currentBatchIndex];
	// END CRITICAL currentBatchIndex //
	// at this point, the current thread should be guaranteed to have the
	//	reference to the correct mesh batch object where the requested instance
	//	is supposed to be appended to //
	// BEGIN CRITICAL meshBatch.baseInstance|instanceCount //
	const size_t instanceIndex =
		meshBatch.baseInstance + meshBatch.instanceCount;
	if (instanceIndex >= instanceModelTranslations.size())
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
			"Instance limit exceeded!\n");
		SDL_assert(false);
		return;
	}
	meshBatch.instanceCount++;
	// END CRITICAL meshBatch.baseInstance|instanceCount //
	// at this point, the current thread should be guaranteed to have exclusive
	//	access to 'instanceIndex' of the instance data buffers //
	instanceModelTranslations[instanceIndex] = translation;
	instanceModelRadians[instanceIndex] = radians;
	instanceModelScale[instanceIndex] = scale;
}
void InstancedMeshCache::draw(VertexArray const& vaTextureless)
{
	if (currentBatchIndex < 0)
	{
		return;
	}
	const size_t totalInstanceCount = 
		batches[currentBatchIndex].baseInstance + 
		batches[currentBatchIndex].instanceCount;
	vbInstanceModelTranslation.update(instanceModelTranslations.data(), 0, totalInstanceCount);
	vbInstanceModelRadians    .update(instanceModelRadians.data()     , 0, totalInstanceCount);
	vbInstanceModelScale      .update(instanceModelScale.data()       , 0, totalInstanceCount);
	VertexArray::use(&vaTextureless, k10::gGlobalUniformBuffer,
					 { &vbPosition, &vbColor, &vbInstanceModelTranslation, 
					 	&vbInstanceModelRadians, &vbInstanceModelScale} );
	for (int b = 0; b < currentBatchIndex + 1; b++)
	{
		MeshBatch const& batch = batches[b];
		MeshData const& meshDat = meshData[batch.meshIndex];
		for (PrimitiveBatch const& pBatch : meshDat.primitiveBatches)
		{
			glDrawArraysInstancedBaseInstance(pBatch.primitiveType,
				pBatch.meshVertexStartIndex, pBatch.meshVertexCount,
				batch.instanceCount, batch.baseInstance);
		}
	}
	currentBatchIndex = -1;
}
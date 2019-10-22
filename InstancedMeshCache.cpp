#include "InstancedMeshCache.h"
bool InstancedMeshCache::create(size_t maxTotalMeshVertexCount,
								size_t maxTotalInstances,
								size_t maxBatchCount,
								u8 maxFrameCascade)
{
	this->maxTotalMeshVertexCount = maxTotalMeshVertexCount;
	this->maxTotalInstances       = maxTotalInstances;
	this->maxBatchCount           = maxBatchCount;
	this->maxFrameCascade         = maxFrameCascade;
	currentMeshInstanceMaximum = 0;
	if (!vbPosition.createFixedSize(maxTotalMeshVertexCount, sizeof(v2f),
									VertexBuffer::MemoryUsage::STATIC))
	{
		SDL_assert(false);
		return false;
	}
	if (!vbColor.createFixedSize(maxTotalMeshVertexCount, sizeof(Color),
								 VertexBuffer::MemoryUsage::STATIC))
	{
		SDL_assert(false);
		return false;
	}
	if (!vbInstanceModelTranslation.createFixedSize(maxTotalInstances * maxFrameCascade,
													sizeof(v2f),
													VertexBuffer::MemoryUsage::STREAM))
	{
		SDL_assert(false);
		return false;
	}
	if (!vbInstanceModelRadians.createFixedSize(maxTotalInstances * maxFrameCascade,
												sizeof(float),
												VertexBuffer::MemoryUsage::STREAM))
	{
		SDL_assert(false);
		return false;
	}
	if (!vbInstanceModelScale.createFixedSize(maxTotalInstances * maxFrameCascade,
											  sizeof(v2f),
											  VertexBuffer::MemoryUsage::STREAM))
	{
		SDL_assert(false);
		return false;
	}
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
	InstancedMeshCache::addMesh(size_t maxInstances, 
								vector<v2f> const& vertexPositions,
								vector<Color> const& vertexColors,
								vector<GLenum> const& primitiveTypes,
								vector<GLint> const& primitiveVertexCounts)
{
	SDL_assert(vertexPositions.size() == vertexColors.size());
	SDL_assert(!primitiveTypes.empty());
	SDL_assert(primitiveTypes.size() == primitiveVertexCounts.size());
	SDL_assert(currentMeshInstanceMaximum + maxInstances <= maxTotalInstances);
	const GLuint baseInstance = meshData.empty() ? 0 :
		meshData.back().baseInstance + 
			meshData.back().instanceCountMax;
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
	const MeshData newMeshDatum
	{
		primitiveBatches,
		baseInstance,// baseInstance
		0, // instanceCount
		static_cast<GLsizei>(maxInstances) // instanceCountMax
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
	const MeshId retVal = static_cast<MeshId>(meshData.size());
	meshData.push_back(newMeshDatum);
	// adding any meshes to the cache will change the total maximum # of 
	//	instances per draw call, so we need to update the cascade data to 
	//	prevent going out of bounds of the instance buffers
	currentMeshInstanceMaximum += maxInstances;
	dataCascadeInstanceOffsetCurrent = 0;
	return retVal;
}
void InstancedMeshCache::updateMeshInstances(MeshId mid, size_t instanceCount,
											 v2f const* instanceModelTranslations,
											 float const* instanceModelRadians,
											 v2f const* instanceModelScales)
{
	OPTICK_EVENT();
	if (mid >= meshData.size())
	{
		SDL_assert(false);
		return;
	}
	MeshData& meshDatum = meshData[mid];
	if (instanceCount > meshDatum.instanceCountMax)
	{
		SDL_assert(false);
		return;
	}
	meshDatum.instanceCount = static_cast<GLsizei>(instanceCount);
	vbInstanceModelTranslation.update(instanceModelTranslations, meshDatum.baseInstance, instanceCount);
	vbInstanceModelRadians    .update(instanceModelRadians     , meshDatum.baseInstance, instanceCount);
	vbInstanceModelScale      .update(instanceModelScales      , meshDatum.baseInstance, instanceCount);
}
bool InstancedMeshCache::mapBuffers()
{
	OPTICK_EVENT();
	if (inMappedState())
	{
		SDL_assert(false);
		return false;
	}
	mappedInstanceModelTranslations = static_cast<v2f*>(
		vbInstanceModelTranslation.mapWriteOnly(0,
												maxTotalInstances* maxFrameCascade, 
												false));
	if (!mappedInstanceModelTranslations)
	{
		SDL_assert(false);
		return false;
	}
	mappedInstanceModelRadians = static_cast<float*>(
		vbInstanceModelRadians.mapWriteOnly(0,
											maxTotalInstances * maxFrameCascade,
											false));
	if (!mappedInstanceModelRadians)
	{
		SDL_assert(false);
		return false;
	}
	mappedInstanceModelScales = static_cast<v2f*>(
		vbInstanceModelScale.mapWriteOnly(0,
										  maxTotalInstances * maxFrameCascade,
										  false));
	if (!mappedInstanceModelScales)
	{
		SDL_assert(false);
		return false;
	}
	return true;
	///////////////////////////////////////////////////////////////////////////
/// 	GLsizei totalInstanceCount = 0;
/// ///	for (MeshData const& meshDatum : meshData)
/// ///	{
/// ///		totalInstanceCount += meshDatum.
/// ///	}
/// 	if (!meshData.empty())
/// 	{
/// 		totalInstanceCount = 
/// 			meshData.back().baseInstance + meshData.back().instanceCountMax;
/// 	}
/// 	SDL_assert(totalInstanceCount > 0);
/// 	const bool orphanBuffers = dataCascadeInstanceOffsetCurrent >= 
/// 		(maxTotalInstances * maxFrameCascade) - totalInstanceCount;
/// 	if (orphanBuffers)
/// 	{
/// 		dataCascadeInstanceOffsetCurrent = 0;
/// 	}
/// ///	SDL_Log("maxTotalInstances=%i totalInstanceCount=%i\n", maxTotalInstances* maxFrameCascade, totalInstanceCount);
/// ///	SDL_Log("dataCascadeInstanceOffsetCurrent=%i\n", dataCascadeInstanceOffsetCurrent);
/// 	dataCascadeInstanceOffsetNext = dataCascadeInstanceOffsetCurrent + totalInstanceCount;
/// 	mappedInstanceModelTranslations = static_cast<v2f*>(
/// 		vbInstanceModelTranslation.mapWriteOnly(dataCascadeInstanceOffsetCurrent,
/// 												totalInstanceCount, orphanBuffers));
/// 	if (!mappedInstanceModelTranslations)
/// 	{
/// 		SDL_assert(false);
/// 		return false;
/// 	}
/// 	mappedInstanceModelRadians = static_cast<float*>(
/// 		vbInstanceModelRadians.mapWriteOnly(dataCascadeInstanceOffsetCurrent,
/// 											totalInstanceCount, orphanBuffers));
/// 	if (!mappedInstanceModelRadians)
/// 	{
/// 		SDL_assert(false);
/// 		return false;
/// 	}
/// 	mappedInstanceModelScales = static_cast<v2f*>(
/// 		vbInstanceModelScale.mapWriteOnly(dataCascadeInstanceOffsetCurrent,
/// 										  totalInstanceCount, orphanBuffers));
/// 	if (!mappedInstanceModelScales)
/// 	{
/// 		SDL_assert(false);
/// 		return false;
/// 	}
/// 	return true;
}
void InstancedMeshCache::postUpdateInstanceJobs(MeshId mid, size_t instanceCount,
												v2f const* instanceModelTranslations,
												float const* instanceModelRadians,
												v2f const* instanceModelScales,
												size_t modelsPerJob)
{
	OPTICK_EVENT();
	if (!inMappedState())
	{
		SDL_assert(false);
		return;
	}
	if (mid >= meshData.size())
	{
		SDL_assert(false);
		return;
	}
	MeshData& meshDatum = meshData[mid];
	if (instanceCount > meshDatum.instanceCountMax)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
			"instanceCount=%i > meshDatum.instanceCountMax=%i\n",
			instanceCount, meshDatum.instanceCountMax);
		SDL_assert(false);
		return;
	}
	meshDatum.instanceCount = static_cast<GLsizei>(instanceCount);
	for (size_t m = 0; m < instanceCount; m += modelsPerJob)
	{
		{
			const JobDataCopyDataV2f jobCopyTranslations
			{
				JobTitle::COPY_DATA_V2F,
				mappedInstanceModelTranslations,
				&instanceModelTranslations[m],
				min(instanceCount - m, size_t(modelsPerJob)),
				dataCascadeInstanceOffsetCurrent + meshDatum.baseInstance + m
			};
			k10::gThreadPool.postJob(jobCopyTranslations);
		}
		{
			const JobDataCopyDataV2f jobCopyScales
			{
				JobTitle::COPY_DATA_V2F,
				mappedInstanceModelScales,
				&instanceModelScales[m],
				min(instanceCount - m, size_t(modelsPerJob)),
				dataCascadeInstanceOffsetCurrent + meshDatum.baseInstance + m
			};
			k10::gThreadPool.postJob(jobCopyScales);
		}
		{
			const JobDataCopyDataFloat jobCopyRadians
			{
				JobTitle::COPY_DATA_FLOAT,
				mappedInstanceModelRadians,
				&instanceModelRadians[m],
				min(instanceCount - m, size_t(modelsPerJob)),
				dataCascadeInstanceOffsetCurrent + meshDatum.baseInstance + m
			};
			k10::gThreadPool.postJob(jobCopyRadians);
		}
	}
}
VertexBuffer::MemoryUnmapResult InstancedMeshCache::unmapBuffers()
{
	OPTICK_EVENT();
	if (!inMappedState())
	{
		return VertexBuffer::MemoryUnmapResult::ERROR;
	}
	VertexBuffer::MemoryUnmapResult retVal =
		VertexBuffer::MemoryUnmapResult::SUCCESS;
	switch (vbInstanceModelTranslation.unmap())
	{
	case VertexBuffer::MemoryUnmapResult::ERROR:
		return VertexBuffer::MemoryUnmapResult::ERROR;
	case VertexBuffer::MemoryUnmapResult::FAILURE:
		retVal = VertexBuffer::MemoryUnmapResult::FAILURE;
	default:
		break;
	}
	switch (vbInstanceModelRadians.unmap())
	{
	case VertexBuffer::MemoryUnmapResult::ERROR:
		return VertexBuffer::MemoryUnmapResult::ERROR;
	case VertexBuffer::MemoryUnmapResult::FAILURE:
		retVal = VertexBuffer::MemoryUnmapResult::FAILURE;
	default:
		break;
	}
	switch (vbInstanceModelScale.unmap())
	{
	case VertexBuffer::MemoryUnmapResult::ERROR:
		return VertexBuffer::MemoryUnmapResult::ERROR;
	case VertexBuffer::MemoryUnmapResult::FAILURE:
		retVal = VertexBuffer::MemoryUnmapResult::FAILURE;
	default:
		break;
	}
	mappedInstanceModelTranslations = nullptr;
	mappedInstanceModelRadians = nullptr;
	mappedInstanceModelScales = nullptr;
///	dataCascadeInstanceOffsetCurrent = dataCascadeInstanceOffsetNext;
	return retVal;
}
void InstancedMeshCache::draw(VertexArray const& vaTextureless)
{
	OPTICK_EVENT();
	VertexArray::use(&vaTextureless, k10::gGlobalUniformBuffer,
					 { &vbPosition, &vbColor, &vbInstanceModelTranslation, 
					 	&vbInstanceModelRadians, &vbInstanceModelScale} );
	for(size_t m = 0; m < meshData.size(); m++)
	{
		MeshData& meshDatum = meshData[m];
		for (PrimitiveBatch const& pBatch : meshDatum.primitiveBatches)
		{
			glDrawArraysInstancedBaseInstance(pBatch.primitiveType,
				pBatch.meshVertexStartIndex, pBatch.meshVertexCount,
				meshDatum.instanceCount, 
				dataCascadeInstanceOffsetCurrent + meshDatum.baseInstance);
		}
		// Ensure that the user must call 'updateMeshInstances' at least once
		//	per frame before 'draw' in order for any instances to get drawn,
		//	since the nature of this class is DYNAMIC instance drawing.
		meshDatum.instanceCount = 0;
	}
///	dataCascadeInstanceOffsetCurrent = dataCascadeInstanceOffsetNext;
	dataCascadeInstanceOffsetCurrent += static_cast<GLuint>(currentMeshInstanceMaximum);
	if (dataCascadeInstanceOffsetCurrent + 
			currentMeshInstanceMaximum > maxTotalInstances * maxFrameCascade)
	{
		dataCascadeInstanceOffsetCurrent = 0;
	}
}
bool InstancedMeshCache::inMappedState() const
{
	return mappedInstanceModelTranslations != nullptr &&
		   mappedInstanceModelRadians      != nullptr &&
		   mappedInstanceModelScales       != nullptr;
}
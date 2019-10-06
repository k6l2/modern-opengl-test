#include "Drawable.h"
Drawable::~Drawable()
{
	for (auto& batch : batches)
	{
		if (batch.indexBufferId)
		{
			glDeleteBuffers(1, &batch.indexBufferId);
		}
	}
	if (vertexBufferId)
	{
		glDeleteBuffers(1, &vertexBufferId);
	}
}
void Drawable::setBatchCount(size_t numBatches)
{
	if (batches.size() == numBatches)
	{
		return;
	}
	// if we're losing batches, clean up index buffers we're about to 
	//	de-allocate //
	if (numBatches < batches.size())
	{
		for (GeometryBatch& batch : batches)
		{
			if (batch.indexBufferId)
			{
				glDeleteBuffers(1, &batch.indexBufferId);
				batch.indexBufferId = NULL;
			}
		}
		batches.resize(numBatches);
	}
	else // numBatches > batches.size()
	{
		const size_t newBatchCount = numBatches - batches.size();
		const size_t prevBatchCount = batches.size();
		batches.resize(numBatches);
		for (size_t b = 0; b < newBatchCount; b++)
		{
			GeometryBatch& batch = batches[prevBatchCount + b];
			glGenBuffers(1, &batch.indexBufferId);
		}
	}
}
size_t Drawable::getBatchCount() const
{
	return batches.size();
}
size_t Drawable::getVertexCount() const
{
	return vertexBufferSize;
}
bool Drawable::setVertexData(vector<VertexData> const& vvd)
{
	if (vvd.empty())
	{
		vertexBufferSize = 0;
		return true;
	}
	if (!vertexBufferId)
	{
		glGenBuffers(1, &vertexBufferId);
	}
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, vvd.size() * sizeof(VertexData),
		&vvd[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	{
		const GLenum errorStateOpenGL = glGetError();
		if (errorStateOpenGL != GL_NO_ERROR)
		{
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
				"OpenGL error! '%s'\n", gluErrorString(errorStateOpenGL));
			return false;
		}
	}
	vertexBufferSize = vvd.size();
	return true;
}
size_t Drawable::getVertexIndexCount(size_t batchId) const
{
	SDL_assert(batchId < batches.size());
	if (batchId >= batches.size())
	{
		return 0;
	}
	return batches[batchId].indexBufferSize;
}
bool Drawable::setVertexIndexData(size_t batchId, vector<Uint32> vid)
{
	if (batchId == 0 && batches.empty())
	{
		setBatchCount(1);
	}
	if (batchId >= batches.size())
	{
		SDL_assert(false);
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
			"batchId=%i is out of bounds! batch count=%i\n",
			batchId, batches.size());
		return false;
	}
	GeometryBatch& batch = batches[batchId];
	if (vid.empty())
	{
		batch.indexBufferSize = 0;
		return true;
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch.indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vid.size() * sizeof(GLuint),
		&vid[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
	{
		const GLenum errorStateOpenGL = glGetError();
		if (errorStateOpenGL != GL_NO_ERROR)
		{
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
				"OpenGL error! '%s'\n", gluErrorString(errorStateOpenGL));
			return false;
		}
	}
	batch.indexBufferSize = vid.size();
	return true;
}
PrimitiveType Drawable::getPrimitiveType(size_t batchId) const
{
	if (batchId >= batches.size())
	{
		SDL_assert(false);
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
			"batchId=%i is out of bounds! batch count=%i\n",
			batchId, batches.size());
		return PrimitiveType::POINTS;
	}
	GeometryBatch const& batch = batches[batchId];
	return batch.primitiveType;
}
void Drawable::setTexture(size_t batchId, Texture const* t)
{
	if (batchId >= batches.size())
	{
		SDL_assert(false);
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
			"batchId=%i is out of bounds! batch count=%i\n",
			batchId, batches.size());
		return;
	}
	GeometryBatch& batch = batches[batchId];
	batch.texture = t;
}
Texture const* Drawable::getTexture(size_t batchId) const
{
	if (batchId >= batches.size())
	{
		SDL_assert(false);
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
			"batchId=%i is out of bounds! batch count=%i\n",
			batchId, batches.size());
		return nullptr;
	}
	GeometryBatch const& batch = batches[batchId];
	return batch.texture;
}
void Drawable::setPrimitiveType(size_t batchId, PrimitiveType pt)
{
	if (batchId >= batches.size())
	{
		SDL_assert(false);
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
			"batchId=%i is out of bounds! batch count=%i\n",
			batchId, batches.size());
		return;
	}
	GeometryBatch& batch = batches[batchId];
	batch.primitiveType = pt;
}
GLuint Drawable::getVertexBufferId() const
{
	return vertexBufferId;
}
GLuint Drawable::getIndexBufferId(size_t batchId) const
{
	if (batchId >= batches.size())
	{
		SDL_assert(false);
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
			"batchId=%i is out of bounds! batch count=%i\n",
			batchId, batches.size());
		return NULL;
	}
	GeometryBatch const& batch = batches[batchId];
	return batch.indexBufferId;
}

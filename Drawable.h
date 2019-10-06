#pragma once
#include "Transformable.h"
#include "Color.h"
struct VertexData
{
	glm::vec2 position;
	glm::vec2 texCoord;
	Color color;
};
enum class BlendFunction : Uint8
{
	ALPHA,
	ADDITIVE,
	MULTIPLICATIVE
};
enum class PrimitiveType : Uint8
{
	POINTS,
	LINE_STRIP,
	TRIANGLE_STRIP
};
struct RenderState
{
	BlendFunction blendFunction = BlendFunction::ALPHA;
};
// Usage Examples:
//
// ----------- Single Batch, Textureless Triangle Strip -------------------
//	Drawable d;
//	d.setVertexData( ... );
//	d.setVertexIndexData(0, ... );
//	renderTarget.draw(d, {});
//
// ----------- Same as above, but additive blending -----------------------
//	"""
//	renderTarget.draw(d, {BlendFunction::ADDITIVE});
//
// ----------- 2 Batches, Textured Triangle Strips ------------------------
//	Drawable d;
//	d.setBatchCount(2);
//	d.setVertexData( ... );
//	d.setVertexIndexData(0, ... );
//	d.setVertexIndexData(1, ... );
//	d.setTexture(0, myTex);
//	d.setTexture(1, myTex2);
//	renderTarget.draw(d, {});
//
// ----------- 2 Batches, Textureless Tristrip w/ thin outline ------------
//	Drawable d;
//	d.setBatchCount(2);
//	d.setVertexData( ... );
//	d.setVertexIndexData(0, ... );
//	d.setVertexIndexData(1, ... );
//	d.setPrimitiveType(1, PrimitiveType::LINE_STRIP);
//	renderTarget.draw(d, {});
class Drawable : public Transformable
{
public:
	// @sdl-port-todo
	//	Allow each Drawable to be provided a memory buffer frequency hint
	//	to allow our gfx drivers to better optimize our bufferData calls
	// @sdl-port-todo
	//	override copy constructors correctly so we can put Drawables into
	//	vectors and stuff w/o worrying about stupid shit happening
	~Drawable();
	void setBatchCount(size_t numBatches);
	size_t getBatchCount() const;
	size_t getVertexCount() const;
	bool setVertexData(vector<VertexData> const& vvd);
	size_t getVertexIndexCount(size_t batchId) const;
	bool setVertexIndexData(size_t batchId, vector<Uint32> vid);
	PrimitiveType getPrimitiveType(size_t batchId) const;
	void setTexture(size_t batchId, class Texture const* t);
	class Texture const* getTexture(size_t batchId) const;
	GLuint getVertexBufferId() const;
	GLuint getIndexBufferId(size_t batchId) const;
	void setPrimitiveType(size_t batchId, PrimitiveType pt);
private:
	struct GeometryBatch
	{
		GLuint indexBufferId = NULL;
		size_t indexBufferSize;
		PrimitiveType primitiveType = PrimitiveType::TRIANGLE_STRIP;
		class Texture const* texture = nullptr;
	};
	GLuint vertexBufferId = NULL;
	// measured in # of vertices, not physical size
	size_t vertexBufferSize;
	vector<GeometryBatch> batches;
};

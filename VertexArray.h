#pragma once
///#include "Color.h"
///struct VertexDataTextured
///{
///	glm::vec2 position;
///	glm::vec2 texCoord;
///	Color color;
///};
///struct VertexDataTextureless
///{
///	glm::vec2 position;
///	Color color;
///};
///union VertexDataUnion
///{
///	VertexDataTextured textured;
///	VertexDataTextureless textureless;
///};
// Unlike the VertexArray class in SFML, which is completely fucking useless,
//	this class is used to manage an OpenGL3+ Vertex Array Object (VAO).
class VertexArray
{
public:
	enum class VertexType : u8
	{
		TEXTURED,
		TEXTURELESS
	};
public:
	//static void use(VertexArray const*const va);
	static bool useTextureless(VertexArray const* const va,
							   class GlobalUniformBuffer const& gub,
							   class VertexBuffer const& vbPosition,
							   class VertexBuffer const& vbColor,
							   class VertexBuffer const& vbModelTranslation,
							   class VertexBuffer const& vbModelRadians);
public:
	bool create(VertexType vt);
	void destroy();
private:
	static const GLuint ATTRIB_LOC_TEXTURELESS_POSITION;
	static const GLuint ATTRIB_LOC_TEXTURELESS_COLOR;
	///static const GLuint ATTRIB_LOC_TEXTURELESS_MODEL2D_COL_0;
	///static const GLuint ATTRIB_LOC_TEXTURELESS_MODEL2D_COL_1;
	///static const GLuint ATTRIB_LOC_TEXTURELESS_MODEL2D_COL_2;
	static const GLuint ATTRIB_LOC_TEXTURELESS_MODEL_TRANSLATION;
	static const GLuint ATTRIB_LOC_TEXTURELESS_MODEL_RADIANS;
	static const GLuint BUFFER_BIND_INDEX_GLOBAL_UNIFORMS;
	static const GLuint BUFFER_BIND_INDEX_TEXTURELESS_POSITION;
	static const GLuint BUFFER_BIND_INDEX_TEXTURELESS_COLOR;
	///static const GLuint BUFFER_BIND_INDEX_TEXTURELESS_MODEL2D;
	static const GLuint BUFFER_BIND_INDEX_TEXTURELESS_MODEL_TRANSLATION;
	static const GLuint BUFFER_BIND_INDEX_TEXTURELESS_MODEL_RADIANS;
private:
	GLuint vertexArrayObject = NULL;
	VertexType vertexType;
};

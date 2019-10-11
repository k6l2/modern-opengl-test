#pragma once
// Unlike the VertexArray class in SFML, which is completely fucking useless,
//	this class is used to manage an OpenGL3+ Vertex Array Object (VAO).
class VertexArray
{
public:
	enum class VertexType : u8
	{
		//TEXTURED,
		//TEXTURELESS,
		TEXTURELESS_MESH_INSTANCES
	};
public:
	static bool use(VertexArray const* const va,
					class GlobalUniformBuffer const& gub,
					vector<class VertexBuffer const*> const& vbs);
public:
	bool create(VertexType vt);
	void destroy();
private:
	static string toString(VertexType vt);
private:
	static const GLuint ATTRIB_LOC_TEXTURELESS_POSITION;
	static const GLuint ATTRIB_LOC_TEXTURELESS_COLOR;
	static const GLuint ATTRIB_LOC_TEXTURELESS_MODEL_TRANSLATION;
	static const GLuint ATTRIB_LOC_TEXTURELESS_MODEL_RADIANS;
	static const GLuint ATTRIB_LOC_TEXTURELESS_MODEL_SCALE;
	static const GLuint BUFFER_BIND_INDEX_GLOBAL_UNIFORMS;
	static const GLuint BUFFER_BIND_INDEX_TEXTURELESS_POSITION;
	static const GLuint BUFFER_BIND_INDEX_TEXTURELESS_COLOR;
	static const GLuint BUFFER_BIND_INDEX_TEXTURELESS_MODEL_TRANSLATION;
	static const GLuint BUFFER_BIND_INDEX_TEXTURELESS_MODEL_RADIANS;
	static const GLuint BUFFER_BIND_INDEX_TEXTURELESS_MODEL_SCALE;
private:
	GLuint vertexArrayObject = NULL;
	VertexType vertexType;
};

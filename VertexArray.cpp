#include "VertexArray.h"
#include "GlobalUniformBuffer.h"
#include "VertexBuffer.h"
const GLuint VertexArray::ATTRIB_LOC_TEXTURELESS_POSITION      = 0;
const GLuint VertexArray::ATTRIB_LOC_TEXTURELESS_COLOR         = 1;
///const GLuint VertexArray::ATTRIB_LOC_TEXTURELESS_MODEL2D_COL_0 = 2;
///const GLuint VertexArray::ATTRIB_LOC_TEXTURELESS_MODEL2D_COL_1 = 3;
///const GLuint VertexArray::ATTRIB_LOC_TEXTURELESS_MODEL2D_COL_2 = 4;
const GLuint VertexArray::ATTRIB_LOC_TEXTURELESS_MODEL_TRANSLATION = 2;
const GLuint VertexArray::ATTRIB_LOC_TEXTURELESS_MODEL_RADIANS     = 3;
const GLuint VertexArray::BUFFER_BIND_INDEX_GLOBAL_UNIFORMS = 0;
// Positions are in model-space, and consequently are unlikely to ever change.
//	Thus, it makes sense to store positions in a separate STATIC buffer.
// Buffer binding index 0 is reserved for the GlobalUniformBuffer~
const GLuint VertexArray::BUFFER_BIND_INDEX_TEXTURELESS_POSITION = 1;
// Colors can potentially change every frame, so it makes sense to store them
//	in a separate DYNAMIC buffer //
const GLuint VertexArray::BUFFER_BIND_INDEX_TEXTURELESS_COLOR    = 2;
// Model matrices are almost GUARANTEED to change every frame, and only vary
//	per INSTANCE (not per vertex).  It might make more sense to store these
//	in a separate buffer and use a special technique to reload the entire
//	thing.  See the following link for more info on that: 
//	https://www.khronos.org/opengl/wiki/Vertex_Specification_Best_Practices#Dynamic_VBO
///const GLuint VertexArray::BUFFER_BIND_INDEX_TEXTURELESS_MODEL2D  = 3;
const GLuint VertexArray::BUFFER_BIND_INDEX_TEXTURELESS_MODEL_TRANSLATION = 3;
const GLuint VertexArray::BUFFER_BIND_INDEX_TEXTURELESS_MODEL_RADIANS     = 4;
bool VertexArray::create(VertexType vt)
{
	if (vertexArrayObject)
	{
		destroy();
	}
	glGenVertexArrays(1, &vertexArrayObject);
	const GLenum oglStatus = glGetError();
	if (oglStatus != GL_NO_ERROR)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
					 "Failed to create VertexArray (vt=%s) OGL Error='%s'\n",
					 vt == VertexType::TEXTURED ? "TEXTURED" :
					 	vt == VertexType::TEXTURELESS ? "TEXTURELESS" :
					 	"UNKNOWN_VERTEX_TYPE",
					 glewGetErrorString(oglStatus));
		return false;
	}
	vertexType = vt;
	return true;
}
void VertexArray::destroy()
{
	glDeleteVertexArrays(1, &vertexArrayObject);
	vertexArrayObject = NULL;
}
//void VertexArray::use(VertexArray const*const va)
//{
//	if (va)
//	{
//		glBindVertexArray(va->vertexArrayObject);
//	}
//	else
//	{
//		glBindVertexArray(NULL);
//	}
//}
bool VertexArray::useTextureless(VertexArray const* const va,
								 GlobalUniformBuffer const& gub,
								 VertexBuffer const& vbPosition,
								 VertexBuffer const& vbColor,
								 VertexBuffer const& vbModelTranslation,
								 VertexBuffer const& vbModelRadians)
{
	if (va)
	{
		glBindVertexArray(va->vertexArrayObject);
		gub.use(BUFFER_BIND_INDEX_GLOBAL_UNIFORMS);
		// vec2 vertexPosition binding //
		vbPosition.bind(BUFFER_BIND_INDEX_TEXTURELESS_POSITION);
		glEnableVertexAttribArray(ATTRIB_LOC_TEXTURELESS_POSITION);
		glVertexAttribFormat(ATTRIB_LOC_TEXTURELESS_POSITION, 
							 2, GL_FLOAT, GL_FALSE, 0);
///		SDL_Log("sizeof(glm::vec2)=%i\n", sizeof(glm::vec2));
		{
			const GLenum oglStatus = glGetError();
			if (oglStatus != GL_NO_ERROR)
			{
				SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
							 "glVertexAttribFormat failed! ");
				switch (oglStatus)
				{
				case GL_INVALID_VALUE:
					SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
								 "GL_INVALID_VALUE\n");
					return false;
					break;
				case GL_INVALID_ENUM:
					SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
								 "GL_INVALID_ENUM\n");
					return false;
					break;
				case GL_INVALID_OPERATION:
					SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
								 "GL_INVALID_OPERATION\n");
					return false;
					break;
				}
				SDL_assert(false);
			}
		}
		glVertexAttribBinding(ATTRIB_LOC_TEXTURELESS_POSITION,
							  BUFFER_BIND_INDEX_TEXTURELESS_POSITION);
		// vec4 vertexColor binding //
		vbColor.bind(BUFFER_BIND_INDEX_TEXTURELESS_COLOR);
		glEnableVertexAttribArray(ATTRIB_LOC_TEXTURELESS_COLOR);
		glVertexAttribFormat(ATTRIB_LOC_TEXTURELESS_COLOR, 
							 4, GL_UNSIGNED_BYTE, GL_TRUE, 0);
		glVertexAttribBinding(ATTRIB_LOC_TEXTURELESS_COLOR, 
							  BUFFER_BIND_INDEX_TEXTURELESS_COLOR);
///		// mat3x2 vertexModel2d binding //
///		vbModel2d.bind(BUFFER_BIND_INDEX_TEXTURELESS_MODEL2D);
///		glEnableVertexAttribArray(ATTRIB_LOC_TEXTURELESS_MODEL2D_COL_0);
///		glEnableVertexAttribArray(ATTRIB_LOC_TEXTURELESS_MODEL2D_COL_1);
///		glEnableVertexAttribArray(ATTRIB_LOC_TEXTURELESS_MODEL2D_COL_2);
///		glVertexAttribFormat(ATTRIB_LOC_TEXTURELESS_MODEL2D_COL_0, 
///							 2, GL_FLOAT, GL_FALSE, 0 * sizeof(glm::mat3x2::col_type));
///		glVertexAttribFormat(ATTRIB_LOC_TEXTURELESS_MODEL2D_COL_1, 
///							 2, GL_FLOAT, GL_FALSE, 1 * sizeof(glm::mat3x2::col_type));
///		glVertexAttribFormat(ATTRIB_LOC_TEXTURELESS_MODEL2D_COL_2, 
///							 2, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::mat3x2::col_type));
///		glVertexAttribBinding(ATTRIB_LOC_TEXTURELESS_MODEL2D_COL_0, 
///							  BUFFER_BIND_INDEX_TEXTURELESS_MODEL2D);
///		glVertexAttribBinding(ATTRIB_LOC_TEXTURELESS_MODEL2D_COL_1, 
///							  BUFFER_BIND_INDEX_TEXTURELESS_MODEL2D);
///		glVertexAttribBinding(ATTRIB_LOC_TEXTURELESS_MODEL2D_COL_2, 
///							  BUFFER_BIND_INDEX_TEXTURELESS_MODEL2D);
///		glVertexBindingDivisor(BUFFER_BIND_INDEX_TEXTURELESS_MODEL2D, 1);
		// modelTranslation binding //
		vbModelTranslation.bind(BUFFER_BIND_INDEX_TEXTURELESS_MODEL_TRANSLATION);
		glEnableVertexAttribArray(ATTRIB_LOC_TEXTURELESS_MODEL_TRANSLATION);
		glVertexAttribFormat(ATTRIB_LOC_TEXTURELESS_MODEL_TRANSLATION,
							 2, GL_FLOAT, GL_FALSE, 0);
		glVertexAttribBinding(ATTRIB_LOC_TEXTURELESS_MODEL_TRANSLATION,
							  BUFFER_BIND_INDEX_TEXTURELESS_MODEL_TRANSLATION);
		glVertexBindingDivisor(BUFFER_BIND_INDEX_TEXTURELESS_MODEL_TRANSLATION, 1);
		// modelRadians binding //
		vbModelRadians.bind(BUFFER_BIND_INDEX_TEXTURELESS_MODEL_RADIANS);
		glEnableVertexAttribArray(ATTRIB_LOC_TEXTURELESS_MODEL_RADIANS);
		glVertexAttribFormat(ATTRIB_LOC_TEXTURELESS_MODEL_RADIANS,
							 1, GL_FLOAT, GL_FALSE, 0);
		glVertexAttribBinding(ATTRIB_LOC_TEXTURELESS_MODEL_RADIANS,
							  BUFFER_BIND_INDEX_TEXTURELESS_MODEL_RADIANS);
		glVertexBindingDivisor(BUFFER_BIND_INDEX_TEXTURELESS_MODEL_RADIANS, 1);
		///glBindVertexBuffer(BUFFER_BIND_INDEX_TEXTURELESS_POSITION, NULL, 0, 0);
		///glBindVertexBuffer(BUFFER_BIND_INDEX_TEXTURELESS_COLOR   , NULL, 0, 0);
		///glBindVertexBuffer(BUFFER_BIND_INDEX_TEXTURELESS_MODEL2D , NULL, 0, 0);
	}
	else
	{
		glBindVertexArray(NULL);
	}
	const GLenum oglStatus = glGetError();
	if (oglStatus != GL_NO_ERROR)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
					 "Failed to use textureless VertexArray. OGL Error='%s'\n",
					 glewGetErrorString(oglStatus));
		SDL_assert(false);
	}
	return true;
}
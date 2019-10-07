#include "GfxProgram.h"
#include "Drawable.h"
#include "Renderer.h"
#include "Texture.h"
bool GfxProgram::load(string const& shaderSpirvBinaryFilenameVertex,
					  string const& shaderSpirvBinaryFilenameFragment, 
					  bool textureless)
{
	this->textureless = textureless;
	// Create and compile the VERTEX shader! //
	const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	if (vertexShader == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
					 "Failed to create shader object!\n");
		return false;
	}
	{
		vector<Uint8> shaderBin = k10::readFile(shaderSpirvBinaryFilenameVertex);
		SDL_assert(glGetError() == GL_NO_ERROR);
		glShaderBinary(1, &vertexShader, k10::SHADER_BINARY_FORMAT,
					   shaderBin.data(), static_cast<GLsizei>(shaderBin.size()));
		const GLenum oglStatus = glGetError();
		if (oglStatus != GL_NO_ERROR)
		{
			switch (oglStatus)
			{
			case GL_INVALID_OPERATION:
				SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
							 "GL_INVALID_OPERATION: more than one shader "
								"handle refers to the same shader object\n");
				break;
			case GL_INVALID_ENUM:
				SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
							 "GL_INVALID_ENUM: 'binaryFormat'(%i) is not an "
								"accepted value \n", 
							 k10::SHADER_BINARY_FORMAT);
				break;
			case GL_INVALID_VALUE:
				SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
							 "GL_INVALID_VALUE: data pointed to by 'binary' "
								"does not match the format specified by "
								"'binaryFormat'(%i), or shader(%i) is "
								"possibly invalid!\n", 
							 k10::SHADER_BINARY_FORMAT, vertexShader);
				break;
			}
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
						 "failed to load pre-compiled shader '%s'\n",
						 shaderSpirvBinaryFilenameVertex);
			printLogShader(vertexShader);
			return false;
		}
		glSpecializeShader(vertexShader, "main", 0, nullptr, nullptr);
		GLint vShaderCompiled = GL_FALSE;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
		if (vShaderCompiled != GL_TRUE)
		{
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
						 "Unable to compile vertex shader %d!\n", vertexShader);
			printLogShader(vertexShader);
			return false;
		}
	}
	// Create and compile the FRAGMENT shader! //
	const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	if (fragmentShader == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
					 "Failed to create shader object!\n");
		return false;
	}
	{
		vector<Uint8> shaderBin = k10::readFile(shaderSpirvBinaryFilenameFragment);
		SDL_assert(glGetError() == GL_NO_ERROR);
		glShaderBinary(1, &fragmentShader, k10::SHADER_BINARY_FORMAT,
					   shaderBin.data(), static_cast<GLsizei>(shaderBin.size()));
		const GLenum oglStatus = glGetError();
		if (oglStatus != GL_NO_ERROR)
		{
			switch (oglStatus)
			{
			case GL_INVALID_OPERATION:
				SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
							 "GL_INVALID_OPERATION: more than one shader "
								"handle refers to the same shader object\n");
				break;
			case GL_INVALID_ENUM:
				SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
							 "GL_INVALID_ENUM: 'binaryFormat'(%i) is not an "
								"accepted value \n", 
							 k10::SHADER_BINARY_FORMAT);
				break;
			case GL_INVALID_VALUE:
				SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
							 "GL_INVALID_VALUE: data pointed to by 'binary' "
								"does not match the format specified by "
								"'binaryFormat'(%i), or shader(%i) is "
								"possibly invalid!\n", 
							 k10::SHADER_BINARY_FORMAT, fragmentShader);
				break;
			}
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
						 "failed to load pre-compiled shader '%s'\n",
						 shaderSpirvBinaryFilenameFragment);
			printLogShader(fragmentShader);
			return false;
		}
		glSpecializeShader(fragmentShader, "main", 0, nullptr, nullptr);
		GLint fShaderCompiled = GL_FALSE;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
		if (fShaderCompiled != GL_TRUE)
		{
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
						 "Unable to compile vertex shader %d!\n", fragmentShader);
			printLogShader(fragmentShader);
			return false;
		}
	}
	// Create the shader program and link the vertex/fragment shaders! //
	programId = glCreateProgram();
	glAttachShader(programId, vertexShader);
	glAttachShader(programId, fragmentShader);
	glLinkProgram(programId);
	GLint programSuccess = GL_TRUE;
	glGetProgramiv(programId, GL_LINK_STATUS, &programSuccess);
	if (programSuccess != GL_TRUE)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO, 
					 "Error linking program %d!\n", programId);
		printLogProgram(programId);
		return false;
	}
	// cleanup shaders since program is now loaded //
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
	// Now that the program is assembled, we can extract attribute locations //
//	projectionMatrixLocation =
//		glGetUniformLocation(programId, "matProjection");
//	if (projectionMatrixLocation == -1)
//	{
//		SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, 
//					"WARNING!  matProjection "
//						"is not a valid glsl program uniform!\n");
//	}
//	viewMatrixLocation =
//		glGetUniformLocation(programId, "matView2d");
//	if (viewMatrixLocation == -1)
//	{
//		SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, 
//					"WARNING!  matView2d "
//						"is not a valid glsl program uniform!\n");
//	}
//	modelMatrixLocation =
//		glGetUniformLocation(programId, "matModel2d");
//	if (modelMatrixLocation == -1)
//	{
//		SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, 
//					"WARNING!  matModel2d "
//						"is not a valid glsl program uniform!\n");
//	}
//	if (!textureless)
//	{
//		textureUnitLocation =
//			glGetUniformLocation(programId, "textureSampler2d");
//		if (textureUnitLocation == -1)
//		{
//			SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, 
//						"WARNING!  textureSampler2d "
//							"is not a valid glsl program uniform!\n");
//		}
//	}
//	vertexPositionLocation =
//		glGetAttribLocation(programId, "vertexPosition");
//	if (vertexPositionLocation == -1)
//	{
//		SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, 
//					"WARNING!  vertexPosition "
//						"is not a valid glsl program uniform!\n");
//	}
//	if (!textureless)
//	{
//		vertexTexCoordLocation =
//			glGetAttribLocation(programId, "vertexTextureCoordinate");
//		if (vertexTexCoordLocation == -1)
//		{
//			SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, 
//						"WARNING!  vertexTextureCoordinate "
//							"is not a valid glsl program uniform!\n");
//		}
//	}
//	vertexColorLocation =
//		glGetAttribLocation(programId, "vertexColor");
//	if (vertexColorLocation == -1)
//	{
//		SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, 
//					"WARNING!  vertexColor "
//						"is not a valid glsl program uniform!\n");
//	}
	return true;
}
void GfxProgram::free()
{
	if (programId != 0)
	{
		glDeleteProgram(programId);
	}
	programId = NULL;
}
void GfxProgram::printLogProgram(GLuint program)
{
	printLogGeneric(program, false);
}
void GfxProgram::printLogShader(GLuint shader)
{
	printLogGeneric(shader, true);
}
void GfxProgram::printLogGeneric(GLuint glObjectId, bool isShader)
{
	if (( isShader && !glIsShader (glObjectId)) ||
		(!isShader && !glIsProgram(glObjectId)))
	{
		SDL_Log("Name %d is not a %s\n",
				glObjectId, isShader?"shader":"program");
		return;
	}
	int infoLogLength = 0;
	int maxLength = infoLogLength;
	if (isShader)
	{
		glGetShaderiv(glObjectId, GL_INFO_LOG_LENGTH, &maxLength);
	}
	else
	{
		glGetProgramiv(glObjectId, GL_INFO_LOG_LENGTH, &maxLength);
	}
	char* infoLog = new char[maxLength];
	if (isShader)
	{
		glGetShaderInfoLog(glObjectId, maxLength, &infoLogLength, infoLog);
	}
	else
	{
		glGetProgramInfoLog(glObjectId, maxLength, &infoLogLength, infoLog);
	}
	if (infoLogLength > 0)
	{
		SDL_Log("%s\n", infoLog);
	}
	delete[] infoLog;
}
void GfxProgram::run(Drawable const& drawable,
					 size_t batchNumber,
					 RenderState const& renderState,
					 glm::mat4 const& projectionMatrix,
					 glm::mat3 const& viewMatrix) const
{
	auto decodeBlendFuncFactorS = [](BlendFunction bf)->GLenum
	{
		switch (bf)
		{
		case BlendFunction::ALPHA:
			return GL_SRC_ALPHA;
		case BlendFunction::ADDITIVE:
			return GL_ONE;
		case BlendFunction::MULTIPLICATIVE:
			return GL_DST_COLOR;
		default:
			SDL_assert(false); break;
		}
		// default to ALPHA blending
		return GL_SRC_ALPHA;
	};
	auto decodeBlendFuncFactorD = [](BlendFunction bf)->GLenum
	{
		switch (bf)
		{
		case BlendFunction::ALPHA:
			return GL_ONE_MINUS_SRC_ALPHA;
		case BlendFunction::ADDITIVE:
			return GL_ONE;
		case BlendFunction::MULTIPLICATIVE:
			return GL_ZERO;
		default:
			SDL_assert(false); break;
		}
		// default to ALPHA blending
		return GL_ONE_MINUS_SRC_ALPHA;
	};
	auto decodeDrawMode = [](PrimitiveType pt)->GLenum
	{
		switch (pt)
		{
		case PrimitiveType::POINTS:
			return GL_POINTS;
		case PrimitiveType::LINE_STRIP:
			return GL_LINE_STRIP;
		case PrimitiveType::TRIANGLE_STRIP:
			return GL_TRIANGLE_STRIP;
		default:
			SDL_assert(false); break;
		}
		// default to TRIANGLE_STRIP
		return GL_TRIANGLE_STRIP;
	};
	const GLenum blendFuncFactorS = 
		decodeBlendFuncFactorS(renderState.blendFunction);
	const GLenum blendFuncFactorD = 
		decodeBlendFuncFactorD(renderState.blendFunction);
	const GLenum drawMode = 
		decodeDrawMode(drawable.getPrimitiveType(batchNumber));
	Texture const*const tex = drawable.getTexture(batchNumber);
	TextureGfxProgramInterface const*const textureProgramInterface =
		tex ? static_cast<TextureGfxProgramInterface const*>(tex) : nullptr;
	glEnable(GL_BLEND);
	glBlendFunc(blendFuncFactorS, blendFuncFactorD);
	glDisable(GL_DEPTH_TEST);
	glUseProgram(programId);
	{
		const GLboolean transposeProjection = GL_FALSE;
		glUniformMatrix4fv(projectionMatrixLocation, 1, transposeProjection,
			glm::value_ptr(projectionMatrix));
	}
	{
		const GLboolean transposeView = GL_FALSE;
		glUniformMatrix3fv(viewMatrixLocation, 1, transposeView,
			glm::value_ptr(viewMatrix));
	}
	{
		const GLboolean transposeModel = GL_FALSE;
		glUniformMatrix3fv(modelMatrixLocation, 1, transposeModel,
			glm::value_ptr(drawable.getTransform()));
	}
	if (!textureless)
	{
		// set the texture sampler uniform to be texture unit 0
		glUniform1i(textureUnitLocation, 0);
		if (textureProgramInterface)
		{
			glBindTexture(GL_TEXTURE_2D, 
						  textureProgramInterface->getTextureId());
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, NULL);
		}
	}
	glEnableVertexAttribArray(vertexPositionLocation);
	if (!textureless)
	{
		glEnableVertexAttribArray(vertexTexCoordLocation);
	}
	glEnableVertexAttribArray(vertexColorLocation);
	// Set vertex data //
	glBindBuffer(GL_ARRAY_BUFFER, drawable.getVertexBufferId());
	glVertexAttribPointer(vertexPositionLocation, 2, GL_FLOAT, GL_FALSE,
						  sizeof(VertexData), 
						  reinterpret_cast<void*>(
							  offsetof(VertexData, position)));
	if (!textureless)
	{
		glVertexAttribPointer(vertexTexCoordLocation, 2, GL_FLOAT, GL_FALSE,
							  sizeof(VertexData), 
							  reinterpret_cast<void*>(
							  	offsetof(VertexData, texCoord)));
	}
	glVertexAttribPointer(vertexColorLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE,
						  sizeof(VertexData), 
						  reinterpret_cast<void*>(
						  	offsetof(VertexData, color)));
	// Set index data and render //
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 
				 drawable.getIndexBufferId(batchNumber));
	glDrawElements(drawMode, 
				   static_cast<GLsizei>(drawable.getVertexIndexCount(batchNumber)),
				   GL_UNSIGNED_INT, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	glDisableVertexAttribArray(vertexPositionLocation);
	if (!textureless)
	{
		glDisableVertexAttribArray(vertexTexCoordLocation);
	}
	glDisableVertexAttribArray(vertexColorLocation);
	glUseProgram(NULL);
}
#ifndef NDEBUG
GLuint GfxProgram::getProgramId() const
{
	return programId;
}
#endif

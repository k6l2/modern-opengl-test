#pragma once
class GfxProgram
{
public:
	bool load(string const& shaderSpirvBinaryFilenameVertex,
			  string const& shaderSpirvBinaryFilenameFragment,
			  bool textureless);
	void free();
	void run(class Drawable const& drawable,
			 size_t batchNumber,
			 struct RenderState const& renderState,
			 glm::mat4 const& projectionMatrix,
			 glm::mat3 const& viewMatrix) const;
#ifndef NDEBUG
	GLuint getProgramId() const;
#endif
private:
	void printLogProgram(GLuint program);
	void printLogShader(GLuint shader);
	void printLogGeneric(GLuint glObjectId, bool isShader);
private:
	GLuint programId = NULL;
	bool textureless;
	// Uniforms //
	GLint projectionMatrixLocation = -1;
	GLint viewMatrixLocation       = -1;
	GLint modelMatrixLocation      = -1;
	GLint textureUnitLocation      = -1;
	// Attributes //
	GLint vertexPositionLocation = -1;
	GLint vertexTexCoordLocation = -1;
	GLint vertexColorLocation    = -1;
};

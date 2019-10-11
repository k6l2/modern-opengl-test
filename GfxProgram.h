#pragma once
// Gfx programs use modern OpenGL (4.6+) compiled shader binaries using a
//	format called SPIR-V.  See the following wiki page:
//	https://www.khronos.org/opengl/wiki/SPIR-V
class GfxProgram
{
public:
	static void use(GfxProgram* gProg);
public:
	bool load(string const& shaderSpirvBinaryFilenameVertex,
			  string const& shaderSpirvBinaryFilenameFragment);
	void free();
///	void run(class Drawable const& drawable,
///			 size_t batchNumber,
///			 struct RenderState const& renderState,
///			 glm::mat4 const& projectionMatrix,
///			 glm::mat3 const& viewMatrix) const;
///#ifndef NDEBUG
///	GLuint getProgramId() const;
///#endif
private:
	void printLogProgram(GLuint program);
	void printLogShader(GLuint shader);
	void printLogGeneric(GLuint glObjectId, bool isShader);
private:
	GLuint programId = NULL;
};

#pragma once
// There should probably either only be one instance of this class at any 
//	given time, or only one instance per Renderer.
// It makes no sense for a Window or a RenderTexture to have multiple 
//	projection matrices.  It might make sense to have multiple views
//	or viewports, but that might be better off managed at the Renderer
//	interface level.  //
// https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL
// https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)
class GlobalUniformBuffer
{
public:
	bool create();
	void destroy();
	void update(glm::mat4 const& projection3d, 
				glm::mat3x2 const& view2d) const;
	void use(GLuint bufferBindingIndex) const;
private:
	GLsizei getSize() const;
private:
	static const GLenum BUFFER_MEMORY_USAGE;
	struct GlobalMatrixBlock
	{
		glm::aligned_mat4 projection3d;
		glm::aligned_mat3 view2d;
	};
private:
	GLuint bufferObject = NULL;
};

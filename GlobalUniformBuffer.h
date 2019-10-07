// https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL
// https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)
#pragma once
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
	struct GlobalMatrixBlock
	{
		glm::aligned_mat4 projection3d;
		glm::aligned_mat3 view2d;
	};
private:
	GLuint bufferObject = NULL;
};

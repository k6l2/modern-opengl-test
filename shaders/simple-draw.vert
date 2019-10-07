#version 460
layout(binding = 0, std140) uniform GlobalMatrixBlock
{
	mat4 projection3d;
	mat3x2 view2d;
};
layout(location = 0) in vec2 vertexPosition;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec2 modelTranslation;
layout(location = 3) in float modelRadians;
//layout(location = 2) in mat3x2 vertexModel2d;
layout(location = 0) out vec4 fragColor;
void main()
{
	// This is how you transform a 2D affine transform matrix into a
	//	3D affine transform matrix.
	//	https://stackoverflow.com/a/5508486/4526664
	mat4 view3d = mat4(
		view2d[0][0], view2d[0][1], 0, 0,
		view2d[1][0], view2d[1][1], 0, 0,
		           0,            0, 1, 0,
		view2d[2][0], view2d[2][1], 0, 1);
//	mat4 model3d = mat4(
//		vertexModel2d[0][0], vertexModel2d[0][1], 0, 0,
//		vertexModel2d[1][0], vertexModel2d[1][1], 0, 0,
//		                  0,                   0, 1, 0,
//		vertexModel2d[2][0], vertexModel2d[2][1], 0, 1);
	mat4 matModelRotation = mat4(
		cos(modelRadians), -sin(modelRadians), 0, 0,
		sin(modelRadians),  cos(modelRadians), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
	mat4 matModelTranslation = mat4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		modelTranslation.x, modelTranslation.y, 0, 1);
	mat4 model3d = matModelTranslation * matModelRotation * mat4(1);
	fragColor   = vertexColor;
	gl_Position = projection3d * view3d * model3d * vec4(vertexPosition, 0, 1);
}
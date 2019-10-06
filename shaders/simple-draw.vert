#version 460
layout(binding = 0, std140) uniform GlobalMatrixBlock
{
	mat4 projection;
	mat3 view2d;
};
layout(binding = 1, std140) uniform ModelMatrixBlock
{
	mat3 model2d;
};
layout(location = 0) in vec2 vertexPosition;
layout(location = 1) in vec4 vertexColor;
layout(location = 0) out vec4 fragColor;
void main()
{
	// This is how you transform a 2D affine transform matrix into a
	//	3D affine transform matrix.
	//	https://stackoverflow.com/a/5508486/4526664
	mat4 view3d = mat4(
		view2d[0][0], view2d[0][1], 0, view2d[0][2],
		view2d[1][0], view2d[1][1], 0, view2d[1][2],
		           0,            0, 1,            0,
		view2d[2][0], view2d[2][1], 0, view2d[2][2]);
	mat4 model3d = mat4(
		model2d[0][0], model2d[0][1], 0, model2d[0][2],
		model2d[1][0], model2d[1][1], 0, model2d[1][2],
		            0,             0, 1,             0,
		model2d[2][0], model2d[2][1], 0, model2d[2][2]);
	fragColor    = vertexColor;
	gl_Position  = projection * view3d * model3d *
		vec4(vertexPosition, 0, 1);
}
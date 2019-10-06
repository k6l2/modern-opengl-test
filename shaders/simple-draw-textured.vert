#version 140
uniform mat4 matProjection;
uniform mat3 matView2d;
uniform mat3 matModel2d;
in vec2 vertexPosition;
in vec2 vertexTextureCoordinate;
in vec4 vertexColor;
out vec2 fragTexCoord;
out vec4 fragColor;
void main()
{
	// This is how you transform a 2D affine transform matrix into a
	//	3D affine transform matrix.
	//	https://stackoverflow.com/a/5508486/4526664
	mat4 view3d = mat4(
		matView2d[0][0], matView2d[0][1], 0, matView2d[0][2],
		matView2d[1][0], matView2d[1][1], 0, matView2d[1][2],
		              0,               0, 1,               0,
		matView2d[2][0], matView2d[2][1], 0, matView2d[2][2]);
	mat4 model3d = mat4(
		matModel2d[0][0], matModel2d[0][1], 0, matModel2d[0][2],
		matModel2d[1][0], matModel2d[1][1], 0, matModel2d[1][2],
		               0,                0, 1,                0,
		matModel2d[2][0], matModel2d[2][1], 0, matModel2d[2][2]);
	fragTexCoord = vertexTextureCoordinate;
	fragColor    = vertexColor;
	gl_Position  = matProjection * view3d * model3d *
		vec4(vertexPosition, 0, 1);
}
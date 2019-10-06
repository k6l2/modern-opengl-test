#include "View.h"
#include "Renderer.h"
View::View(Renderer const& rt)
	: targetSize(rt.getSize())
	, size      (rt.getSize())
{
}
void View::setCenter(v2f const& v)
{
	position = v - 0.5f*size;
}
void View::setPosition(v2f const& v)
{
	position = v;
}
void View::setSize(v2f const& s)
{
	size = s;
}
v2f const& View::getPosition() const
{
	return position;
}
glm::mat3 View::getMatrix() const
{
	const glm::vec2 scaleRatio = targetSize / size;
	return glm::scale(glm::translate(
		glm::mat3(1.f), scaleRatio*-position), scaleRatio);
}

#include "Transformable.h"
void Transformable::setPosition(glm::vec2 const& v2)
{
	position = v2;
}
void Transformable::setScale(glm::vec2 const& v2)
{
	scale = v2;
}
void Transformable::setRadians(float f)
{
	radians = f;
}
void Transformable::setDegrees(float f)
{
	radians = k10::PI / 180 * f;
}
glm::vec2 const& Transformable::getPosition() const
{
	return position;
}
glm::vec2 const& Transformable::getScale() const
{
	return scale;
}
float Transformable::getRadians() const
{
	return radians;
}
float Transformable::getDegrees() const
{
	return radians * 180 / k10::PI;
}
glm::mat3 Transformable::getTransform() const
{
	return glm::scale(glm::rotate(glm::translate(
			glm::mat3(1.f), position), radians), scale);
}

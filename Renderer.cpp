#include "Renderer.h"
#include "GfxProgram.h"
Renderer::Renderer()
	: defaultView(*this)
	, currentView(*this)
{
}
void Renderer::setSize(v2f const& size)
{
	targetSize = size;
	projection = glm::ortho(0.f, size.x, size.y, 0.f);
	defaultView = View{ *this };
	defaultView.setCenter({ 0,0 });
	currentView = defaultView;
}
void Renderer::setView(View const& view)
{
	currentView = view;
}
v2f const& Renderer::getSize() const
{
	return targetSize;
}
View const& Renderer::getView() const
{
	return currentView;
}
View const& Renderer::getDefaultView() const
{
	return defaultView;
}
void Renderer::setProjection(glm::mat4 const& m)
{
	projection = m;
}
glm::mat4 const& Renderer::getProjection() const
{
	return projection;
}

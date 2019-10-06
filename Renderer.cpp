#include "Renderer.h"
#include "GfxProgram.h"
Renderer::Renderer(GfxProgram const*const programTextureless,
				   GfxProgram const*const programTextured)
	: gfxProgramTextureless(programTextureless)
	, gfxProgramTextured   (programTextured)
	, defaultView(*this)
	, currentView(*this)
{
}
void Renderer::setSize(glm::vec2 const& size)
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
glm::vec2 const& Renderer::getSize() const
{
	return targetSize;
}
GfxProgram const*const Renderer::getGfxProgramTextureless() const
{
	return gfxProgramTextureless;
}
GfxProgram const*const Renderer::getGfxProgramTextured() const
{
	return gfxProgramTextured;
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

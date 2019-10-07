#pragma once
#include "Color.h"
#include "Renderer.h"
class Window : public Renderer
{
public:
	static Window* create(char const* title, glm::ivec2 const& size,
						  class GfxProgram const* const programTextureless,
						  class GfxProgram const* const programTextured);
	static void destroy(Window* w);
public:
	Window(class GfxProgram const* const programTextureless,
		   class GfxProgram const* const programTextured);
	virtual void clear(Color const& color) override;
	virtual void draw(class Drawable& drawable, 
					  struct RenderState const& rState) const override;
	void processEvent(SDL_Event const& event);
	void swapBuffer();
private:
	SDL_Window* window = nullptr;
	SDL_GLContext context = NULL;
};

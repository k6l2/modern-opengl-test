#pragma once
#include "Color.h"
#include "Renderer.h"
class Window : public Renderer
{
public:
	static Window* create(char const* title, glm::ivec2 const& size);
	static void destroy(Window* w);
public:
	virtual void clear(Color const& color) override;
	virtual void use() const override;
///	virtual void draw(class Drawable& drawable, 
///					  struct RenderState const& rState) const override;
	void processEvent(SDL_Event const& event);
	void swapBuffer();
	v2f transformToWorldSpace(v2i const& windowSpacePosition) const;
private:
	SDL_Window* window = nullptr;
	SDL_GLContext context = NULL;
};

#pragma once
#include "View.h"
#include "Color.h"
class Renderer
{
public:
	Renderer(class GfxProgram const*const programTextureless,
			 class GfxProgram const*const programTextured);
	virtual void clear(Color const& color) = 0;
	virtual void draw(class Drawable& drawable, 
					  struct RenderState const& rState) const = 0;
	glm::vec2 const& getSize() const;
	void setView(View const& view);
	View const& getView() const;
	View const& getDefaultView() const;
protected:
	virtual void setSize(glm::vec2 const& size);
	void setProjection(glm::mat4 const& m);
	glm::mat4 const& getProjection() const;
	GfxProgram const*const getGfxProgramTextureless() const;
	GfxProgram const*const getGfxProgramTextured() const;
private:
	class GfxProgram const*const gfxProgramTextureless;
	class GfxProgram const*const gfxProgramTextured;
	glm::vec2 targetSize;
	glm::mat4 projection;
	View defaultView;
	View currentView;
};


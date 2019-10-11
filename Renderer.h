#pragma once
#include "View.h"
#include "Color.h"
class Renderer
{
public:
	Renderer();
	virtual void clear(Color const& color) = 0;
	virtual void use() const = 0;
	v2f const& getSize() const;
	void setView(View const& view);
	View const& getView() const;
	View const& getDefaultView() const;
protected:
	virtual void setSize(v2f const& size);
	void setProjection(glm::mat4 const& m);
	glm::mat4 const& getProjection() const;
private:
	v2f targetSize;
	glm::mat4 projection;
	View defaultView;
	View currentView;
};


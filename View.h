#pragma once
class View
{
public:
	View(class Renderer const& rt);
	void setCenter(v2f const& v);
	void setPosition(v2f const& v);
	void setSize(v2f const& s);
	v2f const& getPosition() const;
	glm::mat3 getMatrix() const;
private:
	v2f position;
	v2f targetSize;
	v2f size;
};


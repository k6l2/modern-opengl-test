#pragma once
class Transformable
{
public:
	void setPosition(v2f const& v2);
	void setScale(v2f const& v2);
	void setRadians(float f);
	void setDegrees(float f);
	v2f const& getPosition() const;
	v2f const& getScale() const;
	float getRadians() const;
	float getDegrees() const;
	virtual glm::mat3 getTransform() const;
private:
	v2f position = { 0.f, 0.f };
	v2f scale = { 1.f, 1.f };
	float radians = 0.f;
};

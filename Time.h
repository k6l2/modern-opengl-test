#pragma once
class Time
{
public:
	static const Time ZERO;
public:
	static Time seconds(float amount);
	static Time milliseconds(i32 amount);
	static Time microseconds(i64 amount);
public:
	float seconds() const;
	i32 milliseconds() const;
	i64 microseconds() const;
	Time operator+(Time const& rightHandSide) const;
	Time operator-(Time const& rightHandSide) const;
	Time& operator+=(Time const& rightHandSide);
	Time& operator-=(Time const& rightHandSide);
	bool operator>=(Time const& rightHandSide) const;
	bool operator<=(Time const& rightHandSide) const;
private:
	std::chrono::duration<double> timeDuration = 
		std::chrono::duration<double>(0);
};

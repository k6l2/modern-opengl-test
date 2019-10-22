#include "Time.h"
const Time Time::ZERO;
using std::chrono::duration;
using std::chrono::duration_cast;
Time Time::seconds(float amount)
{
	Time retVal;
	retVal.timeDuration = duration<double>(amount);
	return retVal;
}
Time Time::milliseconds(i32 amount)
{
	Time retVal;
	retVal.timeDuration = 
		duration_cast<duration<double>>(
			duration<i32, std::milli>(amount));
	return retVal;
}
Time Time::microseconds(i64 amount)
{
	Time retVal;
	retVal.timeDuration =
		duration_cast<duration<double>>(
			duration<i64, std::micro>(amount));
	return retVal;
}
float Time::seconds() const
{
	return duration_cast<duration<float>>(timeDuration).count();
}
i32 Time::milliseconds() const
{
	return duration_cast<duration<i32, std::milli>>(timeDuration).count();
}
i64 Time::microseconds() const
{
	return duration_cast<duration<i64, std::micro>>(timeDuration).count();
}
Time Time::operator+(Time const& rightHandSide) const
{
	Time retVal;
	retVal.timeDuration = timeDuration + rightHandSide.timeDuration;
	return retVal;
}
Time Time::operator-(Time const& rightHandSide) const
{
	Time retVal;
	retVal.timeDuration = timeDuration - rightHandSide.timeDuration;
	return retVal;
}
Time& Time::operator+=(Time const& rightHandSide)
{
	timeDuration += rightHandSide.timeDuration;
	return *this;
}
Time& Time::operator-=(Time const& rightHandSide)
{
	timeDuration -= rightHandSide.timeDuration;
	return *this;
}
bool Time::operator>=(Time const& rightHandSide) const
{
	return timeDuration >= rightHandSide.timeDuration;
}
bool Time::operator<=(Time const& rightHandSide) const
{
	return timeDuration <= rightHandSide.timeDuration;
}
bool Time::operator<(Time const& rightHandSide) const
{
	return timeDuration < rightHandSide.timeDuration;
}
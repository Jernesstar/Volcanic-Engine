#pragma once

#include <chrono>

namespace VolcaniCore {

using Clock		 = std::chrono::high_resolution_clock;
using Time_Point = std::chrono::time_point<Clock>;
// TimeStep is SECONDS everywhere. Duration measures elapsed time in seconds.
using Duration	 = std::chrono::duration<float>;

class TimePoint {
public:
	TimePoint() = default;
	TimePoint(const Time_Point& timePoint)
		: m_TimePoint(timePoint) { }
	~TimePoint() = default;

	float operator -(const TimePoint& other) {
		Duration duration = this->m_TimePoint - other.m_TimePoint;
		return (float)duration.count();
	}

private:
	Time_Point m_TimePoint;
};

class TimeStep {
public:
	TimeStep(float time = 0.0f) : m_Time(time) { }

	operator float() const { return m_Time; }

private:
	float m_Time;
};

class Time {
public:
	static TimePoint GetTime() {
		return { Clock::now() };
	}
};

}
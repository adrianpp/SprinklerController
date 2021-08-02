#ifndef TIME_SCHEDULE_HH__
#define TIME_SCHEDULE_HH__

#include <ostream>

struct TimeSchedule {
	int hour; //24 hour time, starting at midnight
	int minute;
	bool operator < (const TimeSchedule& rhs) const
	{
		if( hour < rhs.hour ) return true;
		if( hour > rhs.hour ) return false;
		//hour == rhs.hour at this point
		return( minute < rhs.minute );
	}
	bool operator == (const TimeSchedule& rhs) const
	{
		return (hour == rhs.hour) and (minute == rhs.minute);
	}
	bool operator <= (const TimeSchedule& rhs) const
	{
		return (*this == rhs) or (*this < rhs);
	}
	friend std::ostream& operator << (std::ostream& os, const TimeSchedule& rhs)
	{
		os << rhs.hour << ":" << rhs.minute;
		return os;
	}
};

inline TimeSchedule operator + (const TimeSchedule& lhs, const TimeSchedule& rhs)
{
	auto hour = lhs.hour + rhs.hour;
	auto minute = lhs.minute + rhs.minute;
	hour += minute / 60;
	minute %= 60;
	hour %= 24;
	return {hour,minute};
}

inline bool isBetween(TimeSchedule start, TimeSchedule cur, TimeSchedule end)
{
	if( start <= end )
	{
		return (start <= cur and cur < end);
	}
	else //crosses 24-hour boundary
	{
		return isBetween(start, cur, {24,0}) or isBetween({0,0}, cur, end);
	}
}

#endif /* TIME_SCHEDULE_HH__ */


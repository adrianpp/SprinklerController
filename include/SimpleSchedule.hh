#ifndef SIMPLE_SCHEDULE_HH__
#define SIMPLE_SCHEDULE_HH__

#include "TimeSchedule.hh"
#include <string>
#include "crow/app.h"
#include "crow/json.h"

struct SimpleSetting {
	std::string zone_name;
	bool enabled = false;
	using Duration = TimeSchedule;
	using Time = TimeSchedule;
	Duration duration = {0, 2}; //2 minutes default
	Time start_time = {0,0}; //midnight default start time
	bool manually_running = false;

	template<class...Args>
	SimpleSetting(Args&&... zn) : zone_name(zn...) {}

	bool shouldActivate();
	bool shouldDeactivate();
	std::string generateLayout();
	std::string generateUpdateJS();
	void setRouting(crow::SimpleApp& app);
};

template<int NUM_ZONES>
struct SimpleSchedule {
	SimpleSetting settings[NUM_ZONES];
	template<class...Args>
	SimpleSchedule(Args&&...args) : settings{args...} {}
	std::string generateLayout()
	{
		std::string ret;
		for(auto& S : settings)
			ret += S.generateLayout();
		return ret;
	}
	std::string generateUpdateJS()
	{
		std::string ret;
		for(auto& S : settings)
			ret += S.generateUpdateJS();
		return ret;
	}
	void setRouting(crow::SimpleApp& app)
	{
		for(auto& S : settings)
			S.setRouting(app);
	}
};

#endif /* SIMPLE_SCHEDULE_HH__ */


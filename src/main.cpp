#define CROW_MAIN
#include "crow/mustache.h"
#include "crow/app.h"
#include "TimeSchedule.hh"
#include <chrono>
#include <string>

std::string toTwoDigitTime(int n)
{
	std::string ret;
	if( n < 10 )
		ret += "0";
	ret += std::to_string(n);
	return ret;
}
crow::json::wvalue toJSON(const TimeSchedule& rhs)
{
	crow::json::wvalue ret;
	ret["hour"] = toTwoDigitTime(rhs.hour);
	ret["minute"] = toTwoDigitTime(rhs.minute);
	return ret;
}

struct SimpleSchedule {
	using Duration = TimeSchedule;
	using Time = TimeSchedule;
	static constexpr int NUM_ZONES = 4;
	struct SimpleSetting {
		std::string zone_name;
		bool enabled = false;
		Duration duration = {0, 2}; //2 minutes default
		Time start_time = {0,0}; //midnight default start time
		bool manually_running = false;

		template<class...Args>
		SimpleSetting(Args&&... zn) : zone_name(zn...) {}

		bool shouldActivate()
		{
			const auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			auto tm = localtime(&t);
			Time cur{tm->tm_hour, tm->tm_min};
			auto end = start_time + duration;
			return (enabled and isBetween(start_time, cur, end)) or manually_running;
		}
		bool shouldDeactivate()
		{
			return !shouldActivate() and !manually_running;
		}
		std::string generateLabel(std::string target, std::string text)
		{
			return "<label for='" + zone_name + target + "'>" + text + "</label>\n";
		}
		std::string generateLayout()
		{
			std::string ret;
			ret += "<fieldset id='" + zone_name + "'>\n";
			ret += "<legend>" + zone_name + "</legend>\n";
			ret += generateLabel("_enabled", "Enabled:");
			ret += "<input type='checkbox' id='" + zone_name + "_enabled'>\n";
			ret += "<button id='" + zone_name + "_running'>Running</button><br>\n";
			ret += generateLabel("_start", "Start Time:");
			ret += "<input type='time' id='" + zone_name + "_start'><br>\n";
			ret += generateLabel("_duration_hour", "Duration Hour:");
			ret += "<input type='number' id='" + zone_name + "_duration_hour' min='0' max='24'><br>\n";
			ret += generateLabel("_duration_minute", "Duration Minute:");
			ret += "<input type='number' id='" + zone_name + "_duration_minute' min='0' max='60'><br>\n";
			ret += "</fieldset>\n";
			return ret;
		}
		std::string generateUpdateJS()
		{
			std::string ret;
			ret += "initZone('"+zone_name+"');\n";
			return ret;
		}
		void setRouting(crow::SimpleApp& app)
		{
			app.route_dynamic("/"+zone_name+"/settings")
		    ([&](){
				crow::json::wvalue ret;
				ret["enabled"] = enabled;
				ret["duration"] = toJSON(duration);
				ret["start_time"] = toJSON(start_time);
				ret["running"] = shouldActivate();
				return ret;
			});
			app.route_dynamic("/"+zone_name+"/toggle_enabled")
			([&](){
				enabled = !enabled;
				return "";
			});
			app.route_dynamic("/"+zone_name+"/change_duration")
			([&](const crow::request& req){
				duration.hour = boost::lexical_cast<int>(req.url_params.get("hour"));
				duration.minute = boost::lexical_cast<int>(req.url_params.get("minute"));
				return "";
			});
			app.route_dynamic("/"+zone_name+"/change_start_time")
			([&](const crow::request& req){
				start_time.hour = boost::lexical_cast<int>(req.url_params.get("hour"));
				start_time.minute = boost::lexical_cast<int>(req.url_params.get("minute"));
				return "";
			});
			app.route_dynamic("/"+zone_name+"/toggle_running")
			([&](){
				manually_running = !manually_running;
				return "";
			});
		}
	};
	SimpleSetting settings[NUM_ZONES];
	SimpleSchedule() : settings{"front_main","front_side","rear1","rear2"} {}
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

#include <thread>

int main()
{
	//Crow app initialization
	crow::SimpleApp app;
    app.loglevel(crow::LogLevel::Info);
	crow::mustache::set_base(".");
	
	SimpleSchedule schedule;
	schedule.setRouting(app);

	CROW_ROUTE(app, "/")
    ([&]{
        crow::mustache::context ctx;
		ctx["title"] = "Sprinkler Controller";
		ctx["layout"] = schedule.generateLayout();
		ctx["update_js"] = schedule.generateUpdateJS();
        return crow::mustache::load("default.html").render(ctx);
    });
	std::thread t1([&](){
		bool was_running[SimpleSchedule::NUM_ZONES] = {false};
		[[maybe_unused]] const int pin_num[SimpleSchedule::NUM_ZONES] = {0,1,2,3};
		while(true) {
			int i = 0;
			for(auto& S : schedule.settings)
			{
				if( was_running[i] and S.shouldDeactivate() )
				{
					CROW_LOG_INFO << "Turning off zone " << S.zone_name;
					was_running[i] = false;
				}
				if( !was_running[i] and S.shouldActivate() )
				{
					CROW_LOG_INFO << "Turning on zone " << S.zone_name;
					CROW_LOG_INFO << "Will turn off at " << (S.start_time + S.duration);
					was_running[i] = true;
				}
				++i;
			}
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(1000ms);
		}
	});
	
	app.port(18080).run();
	return 0;
}

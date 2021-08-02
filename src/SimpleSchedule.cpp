#include "SimpleSchedule.hh"
#include <chrono>

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

bool SimpleSetting::shouldActivate()
{
	const auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	auto tm = localtime(&t);
	Time cur{tm->tm_hour, tm->tm_min};
	auto end = start_time + duration;
	return (enabled and isBetween(start_time, cur, end)) or manually_running;
}
std::string generateLabel(const SimpleSetting& ss, std::string target, std::string text)
{
	return "<label for='" + ss.zone_name + target + "'>" + text + "</label>\n";
}
std::string SimpleSetting::generateLayout()
{
	std::string ret;
	ret += "<fieldset id='" + zone_name + "'>\n";
	ret += "<legend>" + zone_name + "</legend>\n";
	ret += generateLabel(*this, "_enabled", "Enabled:");
	ret += "<input type='checkbox' id='" + zone_name + "_enabled'>\n";
	ret += "<button id='" + zone_name + "_running'>Running</button><br>\n";
	ret += generateLabel(*this, "_start", "Start Time:");
	ret += "<input type='time' id='" + zone_name + "_start'><br>\n";
	ret += generateLabel(*this, "_duration_hour", "Duration Hour:");
	ret += "<input type='number' id='" + zone_name + "_duration_hour' min='0' max='24'><br>\n";
	ret += generateLabel(*this, "_duration_minute", "Duration Minute:");
	ret += "<input type='number' id='" + zone_name + "_duration_minute' min='0' max='60'><br>\n";
	ret += "</fieldset>\n";
	return ret;
}
std::string SimpleSetting::generateUpdateJS()
{
	std::string ret;
	ret += "initZone('"+zone_name+"');\n";
	return ret;
}
void SimpleSetting::setRouting(crow::SimpleApp& app)
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


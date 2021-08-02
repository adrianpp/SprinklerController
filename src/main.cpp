#define CROW_MAIN
#include "crow/mustache.h"
#include "crow/app.h"
#include <chrono>
#include "SimpleSchedule.hh"
#include <thread>

int main()
{
	//Crow app initialization
	crow::SimpleApp app;
    app.loglevel(crow::LogLevel::Info);
	crow::mustache::set_base(".");
	constexpr int NUM_ZONES = 4;
	
	SimpleSchedule<NUM_ZONES> schedule{"front_main","front_side","rear1","rear2"};
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
		bool was_running[NUM_ZONES] = {false};
		[[maybe_unused]] const int pin_num[NUM_ZONES] = {0,1,2,3};
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

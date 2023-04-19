#include "json_reader.h"
#include "json_builder.h"

#include <sstream>

using namespace std; 
using namespace json;

namespace json_reader {

    Document LoadJSON(const string& s) {
        std::istringstream strm(s);
        return Load(strm);
    }

    std::string Print(const Node& node) {
        std::ostringstream out;
        Print(json::Document{ node }, out);
        return out.str();
    }

    Dict GetStopInfo(int id, const string& name, catalogue::TransportCatalogue& new_catalogue){
        auto result = json::Builder{};
        result.StartDict().Key("request_id"s).Value(id);
        vector<string> buses = new_catalogue.FindBus(name);
        if (buses.at(0) == "not found"s) {
			result.Key("error_message"s).Value("not found"s);
		}
		else if (buses.at(0) == "no buses"s) {
			Array no_buses = {};
			result.Key("buses"s).Value(no_buses);
		} else {
			Array buses_info;
			for (const auto& bus : buses) {
				buses_info.push_back(json::Node(bus));
			}
			result.Key("buses"s).Value(buses_info);
		}
        
        return result.EndDict().Build().AsMap();
    }
    
    Dict GetBusInfo(int id, const string& name, catalogue::TransportCatalogue& new_catalogue) {
        auto result = json::Builder{};
        result.StartDict().Key("request_id"s).Value(id);
        catalogue::TransportCatalogue::BusInfo bus_info = new_catalogue.GetBusInfo(name);
		if (!bus_info.existence) {
			result.Key("error_message"s).Value("not found"s);
		}
		else {
			result.Key("curvature"s).Value(bus_info.curvature);
			result.Key("route_length"s).Value(static_cast<double>(bus_info.length));
			result.Key("stop_count"s).Value(bus_info.stops_on_route);
			result.Key("unique_stop_count"s).Value(bus_info.unique_stops);
		}
        
        return result.EndDict().Build().AsMap();
    }

	Dict GetAnswer(int id, const string& type, const std::string& name, catalogue::TransportCatalogue& new_catalogue) {
        if(type == "Stop") {
            return GetStopInfo(id, name, new_catalogue);
        } else {
            return GetBusInfo(id, name, new_catalogue);
        }
	}

}  // namespace json_reader
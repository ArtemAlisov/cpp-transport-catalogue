#include "svg.h"
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"
#include "transport_router.h"

using namespace std::literals;
using namespace json;
using namespace json_reader;
using namespace svg;
using namespace map_render;

int main() {
    std::string input_info;
    json::Node node;
    catalogue::TransportCatalogue catalogue;

    // Read data from ctdin
    while (true) {
        std::string str;
        if (!getline(std::cin, str) || (str == "exit"sv)) {
            node = LoadJSON(input_info).GetRoot();
            break;
        }
        input_info += str;
    }

    // put Stops in a catalogue
    for (auto data : node.AsMap().at("base_requests").AsArray()) {
        if (data.AsMap().at("type").AsString() == "Stop") {
            std::string name = data.AsMap().at("name").AsString();
            geo::Coordinates coord = { data.AsMap().at("latitude").AsDouble(), data.AsMap().at("longitude").AsDouble() };
            std::vector<std::pair<std::string, double>> road_distances;
            for (const auto [name, distance] : data.AsMap().at("road_distances").AsMap()) {
                road_distances.push_back({ name, distance.AsDouble() });
            }
            catalogue.AddStop(name, coord, road_distances);
        }
    }

    // put Buses in a catalogue
    MapRender map_render;
    for (auto data : node.AsMap().at("base_requests").AsArray()) {
        if (data.AsMap().at("type").AsString() == "Bus") {
            std::string name = data.AsMap().at("name").AsString();
            std::vector<std::string> stops;
            for (const auto& stop : data.AsMap().at("stops").AsArray()) {
                stops.push_back(stop.AsString());
            }
            bool is_roundtrip = data.AsMap().at("is_roundtrip").AsBool();
            catalogue.AddBus(name);
            catalogue.AddBusRoute(name, stops, is_roundtrip);

            map_render.AddBus(name);  // Fill data for map_render
        }
    }
    
    auto bus_settings = node.AsMap().at("routing_settings").AsMap();
    int bus_velocity = bus_settings.at("bus_velocity").AsInt();
    double bus_wait_time = bus_settings.at("bus_wait_time").AsDouble();
    
    router::TransportRouter transport_router(catalogue);
    transport_router.SetSettings(bus_velocity, bus_wait_time);
    transport_router.MakeGraph();
    graph::Router<double> new_router(transport_router.GetGraph());

    // Fill a map
    RenderSettings render_settings;
    render_settings = FillRenderSettings(node.AsMap().at("render_settings").AsMap());

    std::string new_data = FillSvgDocument(catalogue, render_settings, map_render);
    json::Node doc_new = json::Node(new_data);


    // Return info by stdout
    
    std::vector<json::Node> answer;
    for (auto data : node.AsMap().at("stat_requests").AsArray()) {
        int id = data.AsMap().at("id").AsInt();
        std::string type = data.AsMap().at("type").AsString();
        if (type == "Stop" || type == "Bus") {
            std::string name = data.AsMap().at("name").AsString();
            answer.push_back(GetAnswer(id, type, name, catalogue));
        }
        else if (type == "Map") {
            std::map<std::string, json::Node> cur_answer;
            cur_answer["map"] = doc_new;
            cur_answer["request_id"] = id;
            answer.push_back(cur_answer);
        }
        else if (type =="Route") {
            std::string from = data.AsMap().at("from").AsString();
            std::string to = data.AsMap().at("to").AsString();
            answer.push_back(transport_router.GetGraphData(from, to, id, new_router));
            
        }
        continue;
    }

    std::cout << Print(answer);

}

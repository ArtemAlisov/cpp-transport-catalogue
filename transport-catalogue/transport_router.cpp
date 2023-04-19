#include "transport_router.h"

using namespace std;

namespace router {
    
TransportRouter::TransportRouter(const catalogue::TransportCatalogue& catalogue) {
    catalogue_ = &catalogue;
}

void TransportRouter::SetSettings(int bus_velocity, double bus_wait_time) {
        bus_settings_.bus_velocity = bus_velocity;
        bus_settings_.bus_wait_time = bus_wait_time;
    }
    
    void TransportRouter::MakeStops() {
        size_t id = 0;
        for(const auto& [name, stop] : catalogue_->GetStopsIndex()) {
            stops_ids_[stop->name] = id;
            ids_stops_[id] = stop->name;
            ++id;
            ids_stops_[id] = stop->name + " wait:";
            graph::Edge<double> new_edge;
            new_edge.bus = stop->name;
            new_edge.from = id;
            new_edge.to = id - 1;
            new_edge.weight = bus_settings_.bus_wait_time;
            graph_.AddEdge(new_edge);
            ++id;
        }
    }
    
    void TransportRouter::MakeLocalRoutes(catalogue::TransportCatalogue::Bus* bus, size_t begin, size_t end) {
        int stops = 0;
        double time = 0.;
        double coefficient = 1000. / 60;
        double bus_velocity_meters_per_minute = bus_settings_.bus_velocity * coefficient;
        for (size_t i = begin; i < end; i++) {
            time = 0.;
            auto current_stop = bus->stops.at(i);
            for(size_t j = i + 1; j < end + 1; j++) {
                ++stops;
                auto next_stop = bus->stops.at(j);
                if (catalogue_->GetRealDistances().count(current_stop->name) && catalogue_->GetRealDistances().at(current_stop->name).count(next_stop->name)) {
                    time += 1. * catalogue_->GetRealDistances().at(current_stop->name).at(next_stop->name) / bus_velocity_meters_per_minute;
                }
                else {
                    time += 1. * catalogue_->GetRealDistances().at(next_stop->name).at(current_stop->name) / bus_velocity_meters_per_minute;
                }
                graph::Edge<double> new_edge;
                new_edge.stops = stops;
                new_edge.bus = bus->name;
                new_edge.from = stops_ids_.at(bus->stops.at(i)->name);
                new_edge.to = stops_ids_.at(next_stop->name) + 1;
                new_edge.weight = time;
                graph_.AddEdge(new_edge);
                current_stop = next_stop;
            }
            stops = 0;
        }
    }
    
    void TransportRouter::MakeRoutes() {
        for (auto [name, bus] : catalogue_->GetBusesIndex()) {
            if(!bus->is_roundtrip) {
                MakeLocalRoutes(bus, 0, bus->stops.size()/2);
                MakeLocalRoutes(bus, bus->stops.size()/2, bus->stops.size() - 1);
            } else {
                MakeLocalRoutes(bus, 0, bus->stops.size() - 1);
            }
        }
    }

    void TransportRouter::MakeGraph() {
        graph::DirectedWeightedGraph<double> new_graph(catalogue_->GetStopsIndex().size() * 2);
        graph_ = std::move(new_graph);
        MakeStops();
        MakeRoutes();
    }

    const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
        return graph_;
    }

    json::Dict TransportRouter::GetGraphData(std::string_view from, std::string_view to, int id, graph::Router<double>& new_router) {
        json::Dict result;
        result["request_id"] = id;
        std::vector<std::optional<graph::Router<double>::RouteInfo>> info;
        if(from == to) {
            result["total_time"] = 0;
            json::Array route_info = {};
            result["items"] = route_info;
            return result;
        }
        std::optional<graph::Router<double>::RouteInfo> best = new_router.BuildRoute(stops_ids_.at(from)+1, stops_ids_.at(to)+1);

        if(!best) {
            result["error_message"] = nullptr;  
            // На сколько помню, JSON  по изначальному заданию 10 спринта выводил "null". Возможно это нужно для стандартизации.
            return result;
        }

        json::Dict stops_info = {};
        json::Array route_info = {};
        double total_time = 0.;

        for(auto edge: best.value().edges) {
            if(graph_.GetEdge(edge).from % 2 == 1) {
                stops_info["stop_name"] = graph_.GetEdge(edge).bus;
                stops_info["time"] = graph_.GetEdge(edge).weight;
                stops_info["type"] = "Wait";
                total_time += graph_.GetEdge(edge).weight;
                route_info.push_back(stops_info);
                stops_info = {};
            } else {
                stops_info["bus"] = graph_.GetEdge(edge).bus;
                stops_info["span_count"] = graph_.GetEdge(edge).stops;
                stops_info["time"] = graph_.GetEdge(edge).weight;
                stops_info["type"] = "Bus";
                total_time += graph_.GetEdge(edge).weight;
                route_info.push_back(stops_info);
                stops_info = {};
            }
        }

        result["items"] = route_info;
        result["total_time"] = total_time;
        return result;
    }
}  // namespace router
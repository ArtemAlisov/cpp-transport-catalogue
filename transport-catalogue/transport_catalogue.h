#pragma once

#include "geo.h"
#include "json.h"
#include "graph.h"
#include "router.h"

#include <deque>
#include <unordered_map>
#include <string>
#include <map>
#include <vector>
#include <set>

namespace catalogue {

    class TransportCatalogue {
    public:
        struct Stop {
            std::string name;
            geo::Coordinates coord;
        };

        struct Bus {
            std::string name;
            std::vector<Stop*> stops;
            bool is_roundtrip;
        };

        struct BusInfo {
            bool existence = true;
            int stops_on_route;
            int unique_stops;
            int length = 0;
            double curvature;
        };

        void AddStop(const std::string& name, geo::Coordinates coord,
            const std::vector<std::pair<std::string, double>>& real_dist);
        void AddBus(const std::string& name);
        void AddBusRoute(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip);
        void AddBusCharacteristics(int bus_velocity, double bus_wait_time);
        void MakeVertexesForGraph();
        void MakeNewRoutes(Bus* bus, size_t begin, size_t end);
        void MakeNewGraph();
        
        const graph::DirectedWeightedGraph<double>& GetGraph() const;
        json::Dict GetGraphData(std::string_view from, std::string_view to, int id, graph::Router<double>& new_router);
        std::vector<std::string> FindBus(const std::string_view& stop);
        BusInfo GetBusInfo(const std::string_view& bus);
        std::vector<geo::Coordinates> GetCoordinates() const;
        std::vector<Stop*> GetBusStops(const std::string_view& bus) const;
        bool BusWithRoundtrip(const std::string_view& bus) const;
        Stop* GetStop(const std::string_view& stop) const;
        const std::unordered_map<std::string_view, Stop*>& GetStopsIndex() const;
        const std::unordered_map<std::string_view, Bus*>& GetBusesIndex() const;
        const std::unordered_map<std::string_view, std::unordered_map<std::string, double>>& GetRealDistances() const;

    private:
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, Stop*> stops_index_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Bus*> buses_index_;
        std::unordered_map<std::string_view, std::vector<Bus*>> buses_for_stops_;
        std::unordered_map<std::string_view, std::unordered_map<std::string, double>> real_distances_;
        std::vector<geo::Coordinates> coord_for_buses_;
    };

}  // namespace catalogue
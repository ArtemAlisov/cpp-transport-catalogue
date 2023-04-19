#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"


namespace router {
class TransportRouter {
public:    
    struct BusSettings {
        double bus_wait_time = 0.;
        int bus_velocity = 0;
    };
    
    TransportRouter(const catalogue::TransportCatalogue& catalogue);
    void SetSettings(int bus_velocity, double bus_wait_time);
    void MakeGraph();
    const graph::DirectedWeightedGraph<double>& GetGraph() const;
    json::Dict GetGraphData(std::string_view from, std::string_view to, int id, graph::Router<double>& new_router);
    
private:
    const catalogue::TransportCatalogue* catalogue_;
    graph::DirectedWeightedGraph<double> graph_;
    std::unordered_map<size_t, std::string> ids_stops_;
    std::unordered_map<std::string_view, size_t> stops_ids_;
    BusSettings bus_settings_;
    
    void MakeStops();
    void MakeLocalRoutes(catalogue::TransportCatalogue::Bus* bus, size_t begin, size_t end);
    void MakeRoutes();
    
};
}  // namespace router
#include "transport_catalogue.h"
#include "geo.h"

#include <set>
#include <algorithm>
#include <iostream>

using namespace std;

namespace catalogue {

	void TransportCatalogue::AddStop(const string& name, geo::Coordinates coord,
		const std::vector<pair<string, double>>& real_dist) {
		auto new_stop = new Stop;
		new_stop->name = name;
		new_stop->coord = coord;
		for (const auto& dist : real_dist) {
			real_distances_[new_stop->name][dist.first] = dist.second;
		}
		stops_index_[new_stop->name] = new_stop;
		buses_for_stops_[new_stop->name] = {};
	}

	void TransportCatalogue::AddBus(const string& name) {
		auto new_bus = new Bus;
		new_bus->name = name;
		buses_index_[new_bus->name] = new_bus;

	}

	void TransportCatalogue::AddBusRoute(const string& name, const vector<string>& stops, bool is_roundtrip) {
		auto bus = buses_index_.at(name);
		bus->is_roundtrip = is_roundtrip;

		for (const string& stop : stops) {
			bus->stops.push_back(stops_index_.at(stop));
			buses_for_stops_[stop].push_back(bus);
			coord_for_buses_.push_back(stops_index_.at(stop)->coord);
		}

		size_t length = bus->stops.size();
		if (!is_roundtrip) {
			for (size_t i = length - 1; i > 0; i--) {
				bus->stops.push_back(bus->stops.at(i - 1));
			}
		}
	}
    
	vector<string> TransportCatalogue::FindBus(const string_view& stop) {

		vector<string> result;

		if (buses_for_stops_.count(stop) == 0) {
			result.push_back("not found"s);
			return result;
		}
		if (buses_for_stops_.at(stop).empty()) {
			result.push_back("no buses"s);
			return result;
		}

		// Buses for stop
		set<string> tmp_data;
		for (const auto bus : buses_for_stops_.at(stop)) {
			if (tmp_data.count(bus->name) == 0) {
				tmp_data.insert(bus->name);
				result.push_back(bus->name);
			}
		}
		sort(result.begin(), result.end());

		return result;
	}

	TransportCatalogue::BusInfo TransportCatalogue::GetBusInfo(const string_view& bus) {
		BusInfo result;

		if (buses_index_.count(bus) == 0 || buses_index_.at(bus)->stops.empty()) {
			result.existence = false;
			return result;
		}

		// Calculate distances and curvature
		double distance = 0.;
		double real_distance = 0.;
		Stop* current = buses_index_.at(bus)->stops.at(0);
		set<string> unique_s;
		int first_num = 0;
		for (const auto& stop : buses_index_.at(bus)->stops) {
			unique_s.insert(stop->name);
			if (first_num == 0) {
				first_num++;
				continue;
			}
			if (current == stop) {
				real_distance += real_distances_.at(current->name).at(stop->name);
				continue;
			}
			if (real_distances_.count(current->name) && real_distances_.at(current->name).count(stop->name)) {
				real_distance += real_distances_.at(current->name).at(stop->name);
			}
			else {
				real_distance += real_distances_.at(stop->name).at(current->name);
			}

			distance += geo::ComputeDistance(current->coord, stop->coord);
			current = stop;
		}

		result.unique_stops = unique_s.size();
		result.length = real_distance;
		result.curvature = real_distance / distance;
		result.stops_on_route = buses_index_.at(bus)->stops.size();

		return result;
	}

	vector<geo::Coordinates> TransportCatalogue::GetCoordinates() const {
		return coord_for_buses_;
	}

	vector<TransportCatalogue::Stop*> TransportCatalogue::GetBusStops(const string_view& bus) const {
		vector<TransportCatalogue::Stop*> stops = {};
		if (buses_index_.count(bus) == 0) {
			return stops;
		}
		return buses_index_.at(bus)->stops;
	}

	TransportCatalogue::Stop* TransportCatalogue::GetStop(const string_view& stop) const {
		return stops_index_.at(stop);
	}

	bool TransportCatalogue::BusWithRoundtrip(const string_view& bus) const {
		if (buses_index_.count(bus) == 0) {
			return false;
		}
		return buses_index_.at(bus)->is_roundtrip;
	}
    
    const unordered_map<string_view, TransportCatalogue::Stop*>& TransportCatalogue::GetStopsIndex() const {
        return stops_index_;
    }
    
    const unordered_map<string_view, TransportCatalogue::Bus*>& TransportCatalogue::GetBusesIndex() const {
        return buses_index_;
    }
    
    const unordered_map<string_view, unordered_map<string, double>>& TransportCatalogue::GetRealDistances() const {
        return real_distances_;
    }

}  // namespace catalogue

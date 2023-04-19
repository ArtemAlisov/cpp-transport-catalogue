#pragma once

#include "json.h"
#include "transport_catalogue.h"

#include <string>

namespace json_reader {

    json::Document LoadJSON(const std::string& s);
    std::string Print(const json::Node& node);

    using Dict = std::map<std::string, json::Node>;
    using Array = std::vector<json::Node>;
    Dict GetBusInfo(int id, const std::string& name, catalogue::TransportCatalogue& new_catalogue); 
    Dict GetBusInfo(int id, const std::string& name, catalogue::TransportCatalogue& new_catalogue); 
    Dict GetAnswer(int id, const std::string& type, const std::string& name, catalogue::TransportCatalogue& new_catalogue);
}  // namespace json_reader
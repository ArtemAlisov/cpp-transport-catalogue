#include "map_renderer.h"

#include <iostream>

using namespace std;

namespace map_render {

    void MapRender::AddBus(std::string bus) {
        buses_.insert(bus);
    }

    std::set<std::string> MapRender::GetBuses() const {
        return buses_;
    }

    svg::Color FillCollor(const json::Node& color) {
        if (color.IsString()) {
            return color.AsString();
        }
        else if (color.IsArray()) {
            if (color.AsArray().size() == 3) {
                return svg::Rgb{ static_cast<uint8_t>(color.AsArray().at(0).AsInt()),
                    static_cast<uint8_t>(color.AsArray().at(1).AsInt()),
                    static_cast<uint8_t>(color.AsArray().at(2).AsInt()) };
            }
            else if (color.AsArray().size() == 4) {
                return svg::Rgba{ static_cast<uint8_t>(color.AsArray().at(0).AsInt()),
                    static_cast<uint8_t>(color.AsArray().at(1).AsInt()),
                    static_cast<uint8_t>(color.AsArray().at(2).AsInt()),
                    color.AsArray().at(3).AsDouble() };
            }
        }
        return svg::Rgba{ 0,0,0,1. };
    }


        
        RenderSettings FillRenderSettings(const json::Dict& settings) {
            RenderSettings result;
            if (settings.count("width")) {
                result.width = settings.at("width").AsDouble();
            }
            if (settings.count("height")) {
                result.height = settings.at("height").AsDouble();
            }
            if (settings.count("padding")) {
                result.padding = settings.at("padding").AsDouble();
            }
            if (settings.count("stop_radius")) {
                result.stop_radius = settings.at("stop_radius").AsDouble();
            }
            if (settings.count("line_width")) {
                result.line_width = settings.at("line_width").AsDouble();
            }
            if (settings.count("bus_label_font_size")) {
                result.bus_label_font_size = settings.at("bus_label_font_size").AsInt();
            }
            if (settings.count("bus_label_offset")) {
                json::Array tmp_offset = settings.at("bus_label_offset").AsArray();
                result.bus_label_offset = { tmp_offset[0].AsDouble(), tmp_offset[1].AsDouble() };
            }
            if (settings.count("stop_label_font_size")) {
                result.stop_label_font_size = settings.at("stop_label_font_size").AsInt();
            }
            if (settings.count("stop_label_offset")) {
                json::Array tmp_offset = settings.at("stop_label_offset").AsArray();
                result.stop_label_offset = { tmp_offset[0].AsDouble(), tmp_offset[1].AsDouble() };
            }
            if (settings.count("underlayer_color")) {
                result.underlayer_color = FillCollor(settings.at("underlayer_color"));
            }
            if (settings.count("underlayer_width")) {
                result.underlayer_width = settings.at("underlayer_width").AsDouble();
            }
            if (settings.count("color_palette")) {
                for (auto color : settings.at("color_palette").AsArray()) {
                    result.color_palete.push_back(FillCollor(color));
                }
            }
            return result;
        }

    
    
    vector<svg::Polyline> DrawRoute(const catalogue::TransportCatalogue& catalogue_new, const RenderSettings& render_settings, const MapRender& map_render) {
        vector<svg::Polyline> polylines;
        auto coordinates = catalogue_new.GetCoordinates();
        const SphereProjector proj{ coordinates.begin(),
                                   coordinates.end(),
                                   render_settings.width,
                                   render_settings.height,
                                   render_settings.padding };
        int number = 0;

        // Draw buses
        for (auto bus : map_render.GetBuses()) {
            auto size_palete = render_settings.color_palete.size();
            int i = number % size_palete;
            svg::Polyline polyline;
            polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            polyline.SetStrokeWidth(render_settings.line_width);
            polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            polyline.SetFillColor("none");
            polyline.SetStrokeColor(render_settings.color_palete.at(i));
            if (catalogue_new.GetBusStops(bus).size() == 0) {
                continue;
            }
            for (auto stop : catalogue_new.GetBusStops(bus)) {
                svg::Point new_point = proj(stop->coord);
                polyline.AddPoint(new_point);
            }
            polylines.push_back(std::move(polyline));
            ++number;
        }
        return polylines;
    }
    
    std::pair<svg::Text, svg::Text> FillTextForRoutes(const RenderSettings& render_settings, svg::Point& new_point, const string& bus, int color_index) {
        std::pair<svg::Text, svg::Text> text;
        svg::Text background_text;
        background_text.SetPosition(new_point)
            .SetOffset(render_settings.bus_label_offset)
            .SetFontSize(render_settings.bus_label_font_size)
            .SetFontFamily("Verdana"s)
            .SetFontWeight("bold")
            .SetData(bus)
            .SetFillColor(render_settings.underlayer_color)
            .SetStrokeColor(render_settings.underlayer_color)
            .SetStrokeWidth(render_settings.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg::Text title_text;
        title_text.SetPosition(new_point)
            .SetOffset(render_settings.bus_label_offset)
            .SetFontSize(render_settings.bus_label_font_size)
            .SetFontFamily("Verdana"s)
            .SetFontWeight("bold")
            .SetData(bus)
            .SetFillColor(render_settings.color_palete.at(color_index));
        return {background_text, title_text};
    }
    
    vector<svg::Text> DrawTitlesForRoutes (const catalogue::TransportCatalogue& catalogue_new, const RenderSettings& render_settings, const MapRender& map_render) {
        vector<svg::Text> texts;
        auto coordinates = catalogue_new.GetCoordinates();
        const SphereProjector proj{ coordinates.begin(),
                                   coordinates.end(),
                                   render_settings.width,
                                   render_settings.height,
                                   render_settings.padding };
        int number = 0;
        
        for (auto bus : map_render.GetBuses()) {
            auto size_palete = render_settings.color_palete.size();
            int i = number % size_palete;

            if (catalogue_new.GetBusStops(bus).size() == 0) {
                continue;
            }
            
            int last = catalogue_new.GetBusStops(bus).size() / 2;
            if (catalogue_new.BusWithRoundtrip(bus) ||
                catalogue_new.GetBusStops(bus).at(last)->name == catalogue_new.GetBusStops(bus).at(0)->name) {
                
                geo::Coordinates coord = catalogue_new.GetBusStops(bus).at(0)->coord;
                svg::Point new_point = proj(coord);
                auto text = FillTextForRoutes(render_settings, new_point, bus, i);
                
                texts.push_back(std::move(text.first));
                texts.push_back(std::move(text.second));
            }
            else {
                // for the first stop
                svg::Text background_text_1;
                geo::Coordinates coord = catalogue_new.GetBusStops(bus).at(0)->coord;
                svg::Point new_point = proj(coord);
                auto text_first = FillTextForRoutes(render_settings, new_point, bus, i);
                
                // for the second stop
                geo::Coordinates coord_2 = catalogue_new.GetBusStops(bus).at(last)->coord;
                svg::Point new_point_2 = proj(coord_2);
                auto text_second = FillTextForRoutes(render_settings, new_point_2, bus, i);

                texts.push_back(std::move(text_first.first));
                texts.push_back(std::move(text_first.second));
                texts.push_back(std::move(text_second.first));
                texts.push_back(std::move(text_second.second));
            }
            ++number;
        }
        return texts;
    }
    
    vector<svg::Circle> DrawCirlesForStops (const catalogue::TransportCatalogue& catalogue_new, const RenderSettings& render_settings, std::set<std::string>& unique_stops) {
        vector<svg::Circle> circles;
        auto coordinates = catalogue_new.GetCoordinates();
        const SphereProjector proj{ coordinates.begin(),
                                   coordinates.end(),
                                   render_settings.width,
                                   render_settings.height,
                                   render_settings.padding };
        
        for (auto stop : unique_stops) {
            svg::Circle circle;
            geo::Coordinates coord = catalogue_new.GetStop(stop)->coord;
            svg::Point new_point = proj(coord);

            circle.SetCenter(new_point);
            circle.SetRadius(render_settings.stop_radius);
            circle.SetFillColor("white");

            circles.push_back(std::move(circle));
        }
        return circles;
    }
    
    std::pair<svg::Text, svg::Text> FillTextForStops(const RenderSettings& render_settings, svg::Point& new_point, const string& stop) {
        std::pair<svg::Text, svg::Text> text;
        
        svg::Text background_text;
        background_text.SetPosition(new_point)
            .SetOffset(render_settings.stop_label_offset)
            .SetFontSize(render_settings.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetData(stop)
            .SetFillColor(render_settings.underlayer_color)
            .SetStrokeColor(render_settings.underlayer_color)
            .SetStrokeWidth(render_settings.underlayer_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        svg::Text title_text;
        title_text.SetPosition(new_point)
            .SetOffset(render_settings.stop_label_offset)
            .SetFontSize(render_settings.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetData(stop)
            .SetFillColor("black");
        return {background_text, title_text};
    }
        
    vector<svg::Text> DrawTitlesForStops (const catalogue::TransportCatalogue& catalogue_new, const RenderSettings& render_settings, std::set<std::string>& unique_stops) {
        vector<svg::Text> texts;
        auto coordinates = catalogue_new.GetCoordinates();
        const SphereProjector proj{ coordinates.begin(),
                                   coordinates.end(),
                                   render_settings.width,
                                   render_settings.height,
                                   render_settings.padding };
        for (auto stop : unique_stops) {
            geo::Coordinates coord = catalogue_new.GetStop(stop)->coord;
            svg::Point new_point = proj(coord);
            std::pair<svg::Text, svg::Text> text = FillTextForStops(render_settings, new_point, stop);
            texts.push_back(std::move(text.first));
            texts.push_back(std::move(text.second));
        }
        return texts;
    }
    
    std::string FillSvgDocument(const catalogue::TransportCatalogue& catalogue_new, const RenderSettings& render_settings, const MapRender& map_render) {
        
        svg::Document doc;
        for(auto polyline : DrawRoute(catalogue_new, render_settings, map_render)){
            doc.Add(polyline);
        }

        for(auto text : DrawTitlesForRoutes(catalogue_new, render_settings, map_render)){
            doc.Add(text);
        }

        std::set<std::string> unique_stops;
        for (auto bus : map_render.GetBuses()) {
            if (catalogue_new.GetBusStops(bus).size() == 0) {
                continue;
            }
            for (auto stop : catalogue_new.GetBusStops(bus)) {
                unique_stops.insert(stop->name);
            }
        }
        
        for(auto circle : DrawCirlesForStops(catalogue_new, render_settings, unique_stops)){
            doc.Add(circle);
        }
        
        for(auto text : DrawTitlesForStops(catalogue_new, render_settings, unique_stops)){
            doc.Add(text);
        }
        
        std::ostringstream str_new;
        doc.Render(str_new);
        std::string new_data = str_new.str();
        return new_data;
    }
    

}  // namespace map_render

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}
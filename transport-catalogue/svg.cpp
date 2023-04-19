#include "svg.h"

#include <iomanip>

namespace svg {
    std::ostream& operator<<(std::ostream& out, Color output) {
        std::ostringstream strm;
        visit(OstreamPrinter{ strm }, output);
        out << strm.str();
        return out;
    }


    std::ostream& operator<<(std::ostream& out, const std::optional<StrokeLineJoin> output) {
        if (output == StrokeLineJoin::ARCS) {
            out << "arcs";
        }
        else if (output == StrokeLineJoin::BEVEL) {
            out << "bevel";
        }
        else if (output == StrokeLineJoin::MITER) {
            out << "miter";
        }
        else if (output == StrokeLineJoin::MITER_CLIP) {
            out << "miter-clip";
        }
        else if (output == StrokeLineJoin::ROUND) {
            out << "round";
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const std::optional<StrokeLineCap> output) {
        if (output == StrokeLineCap::BUTT) {
            out << "butt";
        }
        else if (output == StrokeLineCap::ROUND) {
            out << "round";
        }
        else if (output == StrokeLineCap::SQUARE) {
            out << "square";
        }
        return out;
    }


    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point) {
        vertexes_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << std::setprecision(6);
        out << "<polyline points=\""sv;
        int counter = 0;
        for (Point vertex : vertexes_) {
            if (counter == 0) {
                out << vertex.x << ","sv << vertex.y;
                ++counter;
            }
            else {
                out << " "sv << vertex.x << ","sv << vertex.y;
            }
        }
        out << "\"";
        RenderAttrs(out);
        out << "/>"sv;
    }


    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    Text& Text::SetFontSize(std::uint32_t font_size) {
        font_size_ = font_size;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }
    //  <text x="35" y="20" dx="0" dy="6" font-size="12" font-family="Verdana" font-weight="bold">Hello C++</text>
    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << std::setprecision(6);
        out << "<text";
        RenderAttrs(out);
        out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv;
        out << font_size_;
        if (font_family_.size()) {
            out << "\" font-family=\""sv << font_family_;
        }
        if (font_weight_.size()) {
            out << "\" font-weight=\""sv << font_weight_;
        }
        out << "\">";
        if (data_.size()) {
            out << data_;
        }
        out << "</text>";
    }
    /*Point position_;
      Point offset_;
      std::string name_;
      std::string data_;
      std::uint32_t size_;
      std::string font_weight_;*/

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext ctx(out, 2, 2);
        for (size_t i = 0; i < objects_.size(); i++) {
            objects_.at(i)->Render(ctx);
        }
        out << "</svg>"sv;
    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

}  // namespace svg
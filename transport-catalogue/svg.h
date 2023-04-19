#pragma once
#define _USE_MATH_DEFINES 

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cmath>
#include <optional>
#include <iomanip>
#include <sstream>
#include <variant>

namespace svg {

    // Объявив в заголовочном файле константу со спецификатором inline,
    // мы сделаем так, что она будет одной на все единицы трансляции,
    // которые подключают этот заголовок.
    // В противном случае каждая единица трансляции будет использовать свою копию этой константы

    struct Rgb {
        Rgb(uint8_t r, uint8_t g, uint8_t b) {
            red = r;
            green = g;
            blue = b;
        }
        Rgb() = default;
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba(uint8_t r, uint8_t g, uint8_t b, double a = 1.0) {
            red = r;
            green = g;
            blue = b;
            opacity = a;
        }
        Rgba() = default;
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };


    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const Color NoneColor{ "none" };

    struct OstreamPrinter {
        std::ostream& out;

        void operator()(std::monostate) const {
            out << "none";
        }
        void operator()(std::string color) const {
            out << color;
        }
        void operator()(Rgb rgb) const {
            out << "rgb(" << int(rgb.red) << "," << int(rgb.green) << "," << int(rgb.blue) << ")";
        }
        void operator()(Rgba rgba) const {
            out << "rgba(" << int(rgba.red) << "," << int(rgba.green) << "," << int(rgba.blue) << "," << rgba.opacity << ")";
        }
    };

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, const std::variant<std::monostate, std::string, Rgb, Rgba> output);

    std::ostream& operator<<(std::ostream& out, const std::optional<StrokeLineJoin> output);

    std::ostream& operator<<(std::ostream& out, const std::optional<StrokeLineCap> output);

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeWidth(double stroke_width) {
            stroke_width_ = std::move(stroke_width);
            return AsOwner();
        }
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_linecap_ = std::move(line_cap);
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_linejoin_ = std::move(line_join);
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;
            out << std::setprecision(6);
            if (!fill_ && !stroke_ && !stroke_width_ && !stroke_linecap_ && !stroke_linejoin_) { out << " "sv; }
            if (fill_) {
                out << " fill=\""sv << *fill_ << "\""sv;
            }
            if (stroke_) {
                out << " stroke=\""sv << *stroke_ << "\""sv;
            }

            if (stroke_width_ != 0.0) {
                out << " stroke-width=\""sv << stroke_width_ << "\""sv;
            }
            if (stroke_linecap_) {
                out << " stroke-linecap=\""sv << stroke_linecap_ << "\""sv;
            }
            if (stroke_linejoin_) {
                out << " stroke-linejoin=\""sv << stroke_linejoin_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_;
        std::optional<Color> stroke_;
        double stroke_width_ = 0.0;
        std::optional<StrokeLineCap> stroke_linecap_;
        std::optional<StrokeLineJoin> stroke_linejoin_;
    };

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> vertexes_;
    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

        // Прочие данные и методы, необходимые для реализации элемента <text>
    private:
        Point position_;
        Point offset_;
        std::uint32_t font_size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;

        void RenderObject(const RenderContext& context) const override;
    };

    class ObjectContainer {
    public:
        template <typename Obj>
        void Add(Obj obj) {
            std::unique_ptr<Obj> obj_new = std::make_unique<Obj>(std::move(obj));
            AddPtr(std::move(obj_new));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

        virtual ~ObjectContainer() = default;


    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };

    class Document : public ObjectContainer {
    public:
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        void Render(std::ostream& out) const;

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

}  // namespace svg

namespace shapes {
    using Color = std::string;

    class Triangle : public svg::Drawable {
    public:
        Triangle(svg::Point point_1, svg::Point point_2, svg::Point point_3)
            : point_1_(point_1)
            , point_2_(point_2)
            , point_3_(point_3) {
        }

        // Реализует метод Draw интерфейса svg::Drawable
        void Draw(svg::ObjectContainer& container) const override {
            container.Add(svg::Polyline().AddPoint(point_1_).AddPoint(point_2_).AddPoint(point_3_).AddPoint(point_1_));
        }

    private:
        svg::Point point_1_, point_2_, point_3_;
    };


    class Star : public svg::Drawable {
    public:
        Star(svg::Point center, double outer_rad, double inner_rad, int num_rays)
            : center_(center)
            , outer_rad_(outer_rad)
            , inner_rad_(inner_rad)
            , num_rays_(num_rays) {
        }
        Color color = "red";
        Color stroke = "black";

        // Реализует метод Draw интерфейса svg::Drawable
        void Draw(svg::ObjectContainer& container) const override {
            using namespace svg;
            Polyline polyline;
            for (int i = 0; i <= num_rays_; ++i) {
                double angle = 2 * M_PI * (i % num_rays_) / num_rays_;
                polyline.AddPoint({ center_.x + outer_rad_ * sin(angle), center_.y - outer_rad_ * cos(angle) });
                if (i == num_rays_) {
                    break;
                }
                angle += M_PI / num_rays_;
                polyline.AddPoint({ center_.x + inner_rad_ * sin(angle), center_.y - inner_rad_ * cos(angle) });
            }
            container.Add(polyline.SetFillColor(color).SetStrokeColor(stroke));
        }

    private:
        svg::Point center_;
        double outer_rad_;
        double inner_rad_;
        int num_rays_;
    };

    class Snowman : public svg::Drawable {
    public:
        Snowman(svg::Point point, double radius)
            :point_(point), radius_(radius)
        {
        }

        void Draw(svg::ObjectContainer& container) const override {
            using namespace svg;
            Color color = "rgb(240,240,240)";
            Color stroke = "black";
            Point point_2 = { point_.x, point_.y + 5 * radius_ };
            container.Add(Circle().SetCenter(point_2).SetRadius(radius_ * 2).SetFillColor(color).SetStrokeColor(stroke));
            Point point_1 = { point_.x, point_.y + 2 * radius_ };
            container.Add(Circle().SetCenter(point_1).SetRadius(radius_ * 1.5).SetFillColor(color).SetStrokeColor(stroke));
            container.Add(Circle().SetCenter(point_).SetRadius(radius_).SetFillColor(color).SetStrokeColor(stroke));
        }

    private:
        svg::Point point_;
        double radius_;
    };


} // namespace shapes 


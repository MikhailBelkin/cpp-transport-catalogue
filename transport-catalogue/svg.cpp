#include "svg.h"
#include <string_view>
#include <algorithm>



std::ostream& operator <<(std::ostream& os, svg::StrokeLineCap lc) {
    switch (lc)
    {
    case svg::StrokeLineCap::BUTT:
        os << "butt";
        break;
    case svg::StrokeLineCap::ROUND:
        os << "round";
        break;
    case svg::StrokeLineCap::SQUARE:
        os << "square";
        break;
    default:
        break;
    }
    return os;
}


std::ostream& operator <<(std::ostream& os, svg::StrokeLineJoin lj) {
    switch (lj)
    {
    case svg::StrokeLineJoin::ARCS:
        os << "arcs";
        break;
    case svg::StrokeLineJoin::BEVEL:
        os << "bevel";
        break;
    case svg::StrokeLineJoin::MITER:
        os << "miter";
        break;
    case svg::StrokeLineJoin::MITER_CLIP:
        os << "miter-clip";
        break;
    case svg::StrokeLineJoin::ROUND:
        os << "round";
        break;
    default:
        break;
    }


    return os;
}

std::ostream& operator <<(std::ostream& os, svg::Color color) {
    std::visit(svg::ColorPrint{ os }, color);
    return os;
}



namespace svg {

    using namespace std::literals;
    using namespace std::literals::string_view_literals;



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

        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);

        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;

    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool space = false;
        for (auto point : points_) {
            if (!space) {
                space = true;
            }
            else {
                out << " ";
            }
            out << point.x << ","sv << point.y;
        }
        out << "\""sv;

        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);

        out << "/>"sv;

    }


    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    // Прочие данные и методы, необходимые для реализации элемента <text>

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text";
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << " x=\"" << pos_.x << "\" y=\"" << pos_.y << "\"";
        out << " dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\"";
        out << " font-size=\"" << font_size_ << "\"";
        if (font_family_ != "") out << " font-family=\"" << font_family_ << "\"";
        
        if (font_weight_ != "") out << " font-weight=\"" << font_weight_ << "\"";
        
        out << ">" << data_ << "</text>";
    }

    /*template <typename Obj>
    void Add(Obj& object) {
        AddPtr(std::make_unique<ObjectType>(std::move(std::forward<ObjectType>(object))));
    }*/



    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));

    }


    // Выводит в ostream svg-представление документа
    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext ctx(out, 2, 2);
        for (auto& obj : objects_) {
            obj.get()->Render(ctx);

        }

        out << "</svg>"sv;
    }


}  // namespace svg
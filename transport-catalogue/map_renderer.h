#pragma once


#include "geo.h"
#include "svg.h"


#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <utility>


namespace map_render {

    
    

    struct MapRenderSettings {
        double width;
        double height;
        double padding;
        double line_with;
        double stop_radius;

        int bus_label_font_size;
        std::pair<double, double> bus_label_offset;

        int stop_label_font_size;
        std::pair<double, double> stop_label_offset;

        svg::Color underlayer_color;

        double underlayer_width;
        std::vector <svg::Color> color_palette;

    };

    inline const double EPSILON = 1e-6;
    bool IsZero(double value);
    
    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };



    class MapRender {
            
        public:

            struct All_Routes {
                struct stop {
                    std::string name;
                    geo::Coordinates coordinates_;
                    bool operator==(const stop& s) const {
                        return  this->name == s.name &&
                            this->coordinates_.lat == s.coordinates_.lat &&
                            this->coordinates_.lng == s.coordinates_.lng;
                    }
                };
                struct bus {
                    std::string name;
                    std::vector<stop> stops;
                    bool is_round;
                };


                std::vector<bus> buses;

                double track_lenght = 0;
                double track_distance = 0;
                std::vector<map_render::SphereProjector> sphere_coord;


            };


           MapRenderSettings map_set;
           All_Routes all_routes;
           

           std::string RenderMap();

    private:
        svg::Polyline RenderBus(const MapRender::All_Routes::bus& bus, size_t color_num, std::vector <All_Routes::stop>& all_stops);
        std::pair<svg::Text, svg::Text > RenderBusNames(const MapRender::All_Routes::bus& bus, size_t color_num, geo::Coordinates coord);
        std::pair<svg::Text, svg::Text> RenderStopNames(const MapRender::All_Routes::stop& stop);
        svg::Circle RenderStopCirlce(const MapRender::All_Routes::stop& stop);

    };

}

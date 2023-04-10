#include "map_renderer.h"
#include "request_handler.h"
#include "svg.h"

#include <sstream>

using namespace request_queue;

namespace map_render {
    bool IsZero(double value)
    {
        return std::abs(value) < EPSILON;
    }

	svg::Polyline MapRender::RenderBus(const MapRender::All_Routes::bus& bus, size_t color_num, std::vector <All_Routes::stop>& all_stops) {
		svg::Polyline polyline;
		polyline.SetStrokeColor(map_set.color_palette[color_num]);
		polyline.SetFillColor(svg::NoneColor);
		polyline.SetStrokeWidth(map_set.line_with);
		polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		for (auto stop : bus.stops) {
			const svg::Point coord = all_routes.sphere_coord[0](stop.coordinates_);
			polyline.AddPoint(coord);
			if (std::find(all_stops.begin(), all_stops.end(), stop) == all_stops.end()) {
				all_stops.push_back(stop);
			}
		}
		return polyline;
	}

	std::pair<svg::Text, svg::Text> MapRender::RenderBusNames(const MapRender::All_Routes::bus& bus, size_t color_num, geo::Coordinates coord) {
	

		svg::Text first_stop_text;
		svg::Text first_stop_undertext;

		first_stop_text.SetData(bus.name);
		first_stop_undertext.SetData(bus.name);

		first_stop_undertext.SetFillColor(map_set.underlayer_color);
		first_stop_text.SetFillColor(map_set.color_palette[color_num]);

		first_stop_undertext.SetStrokeColor(map_set.underlayer_color);
		first_stop_undertext.SetStrokeWidth(map_set.underlayer_width);
		first_stop_undertext.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		first_stop_undertext.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		first_stop_text.SetFontSize(map_set.bus_label_font_size);
		first_stop_undertext.SetFontSize(map_set.bus_label_font_size);

		first_stop_text.SetFontFamily("Verdana");
		first_stop_undertext.SetFontFamily("Verdana");

		first_stop_text.SetFontWeight("bold");
		first_stop_undertext.SetFontWeight("bold");

		svg::Point coord1 = all_routes.sphere_coord[0](coord);
		first_stop_text.SetPosition(coord1);
		first_stop_undertext.SetPosition(coord1);

		coord1.x = map_set.bus_label_offset.first;
		coord1.y = map_set.bus_label_offset.second;
		first_stop_text.SetOffset(coord1);
		first_stop_undertext.SetOffset(coord1);
		return std::make_pair(first_stop_text, first_stop_undertext);


	}


	svg::Circle MapRender::RenderStopCirlce(const MapRender::All_Routes::stop& stop) {
		svg::Circle stop_circle;
		svg::Point coord = all_routes.sphere_coord[0](stop.coordinates_);
		stop_circle.SetCenter(coord);
		stop_circle.SetRadius(map_set.stop_radius);
		stop_circle.SetFillColor("white");
		return stop_circle;
	}


	std::pair<svg::Text, svg::Text> MapRender::RenderStopNames(const MapRender::All_Routes::stop& stop) {
		svg::Text stop_text;
		svg::Text stop_undertext;

		stop_text.SetData(stop.name);
		stop_undertext.SetData(stop.name);

		stop_undertext.SetFillColor(map_set.underlayer_color);
		stop_text.SetFillColor("black");

		stop_undertext.SetStrokeColor(map_set.underlayer_color);
		stop_undertext.SetStrokeWidth(map_set.underlayer_width);
		stop_undertext.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		stop_undertext.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		stop_text.SetFontSize(map_set.stop_label_font_size);
		stop_undertext.SetFontSize(map_set.stop_label_font_size);

		stop_text.SetFontFamily("Verdana");
		stop_undertext.SetFontFamily("Verdana");

		svg::Point coord1 = all_routes.sphere_coord[0](stop.coordinates_);
		stop_text.SetPosition(coord1);
		stop_undertext.SetPosition(coord1);

		coord1.x = map_set.stop_label_offset.first;
		coord1.y = map_set.stop_label_offset.second;
		stop_text.SetOffset(coord1);
		stop_undertext.SetOffset(coord1);
		return std::make_pair(stop_text, stop_undertext);
	}

    std::string MapRender::RenderMap() {

		

		svg::Document doc;
		std::vector <All_Routes::stop> all_stops;
		size_t color_num = 0;
		for (auto bus : all_routes.buses) {

			// рисуем маршруты
			doc.Add(RenderBus(bus, color_num, all_stops));

			if ((color_num + 1) < map_set.color_palette.size()) {
				color_num++;
			}
			else {
				color_num = 0;
			}


		}

		color_num = 0;// заново считаем те же цвета для остановок

		// отрисовка названий маршрутов
		for (auto bus : all_routes.buses) {
			auto first_stop = bus.stops[0];
			std::pair<svg::Text, svg::Text> route_name = MapRender::RenderBusNames(bus, color_num, first_stop.coordinates_);
			doc.Add(route_name.second);
			doc.Add(route_name.first);

			auto last_stop = bus.stops[bus.stops.size() / 2]; // так как маршрут кольцевой - последняя остановка в середине
			if (!bus.is_round && !(first_stop == last_stop)) {

				std::pair<svg::Text, svg::Text> route_name = MapRender::RenderBusNames(bus, color_num, last_stop.coordinates_);
				doc.Add(route_name.first);
				doc.Add(route_name.second);

			}


			if ((color_num + 1) < map_set.color_palette.size()) {
				color_num++;
			}
			else {
				color_num = 0;
			}
		}

		// выводим все остановки
		{
			std::sort(all_stops.begin(), all_stops.end(), [](All_Routes::stop lhs, All_Routes::stop rhs)
				{
					return lhs.name < rhs.name;
				});

			for (auto stop : all_stops) {
				doc.Add(RenderStopCirlce(stop));
			}



		}

		// выводим названия остановок
		{
			for (auto stop : all_stops) {
				std::pair<svg::Text, svg::Text> stop_name = RenderStopNames(stop);
				doc.Add(stop_name.second);
				doc.Add(stop_name.first);
			}
		}


		std::ostringstream temp_stream;
		doc.Render(temp_stream);
		return  temp_stream.str();
    }


}


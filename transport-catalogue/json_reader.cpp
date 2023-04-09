#include "json_reader.h"
#include <cassert>
#include "svg.h"
#include "map_renderer.h"
#include <sstream>


using namespace transport_catalogue;
using namespace request_queue;
using namespace transport;

namespace transport_catalogue_output_json {
	using namespace std::literals;
	json::Dict BusInfoOutput( transport_catalogue::request_queue::RequestQueue::QueryResult& element) {
		json::Dict info;

		info["request_id"s] = element.id;
		//os << "Bus " << element.name_ << ": ";
		if (element.found) {
			std::set<std::string> uniq(element.stops.begin(), element.stops.end());
			info["stop_count"] = static_cast<int>(element.stops.size());
			info["unique_stop_count"] = static_cast<int>(uniq.size());
			info["route_length"] = element.track_distance;
			info["curvature"] = double(element.track_distance) / double(element.track_lenght);
				
		}
		else {
			info["error_message"s]= "not found"s;
		}
		return info;
	}



	json::Dict StopOutputInfo( transport_catalogue::request_queue::RequestQueue::QueryResult& element) {
		json::Dict info;
		json::Array buses;

		info["request_id"s] = element.id;

		//os << "Stop " << element.name_ << ": ";
		if (element.found) {
			if (element.buses.size() > 0) {
				std::sort(element.buses.begin(), element.buses.end());
				//os << "buses";
			}
			for (auto bus : element.buses) {
					 buses.push_back(bus);
			}
			info["buses"s] = buses;
			
			
		}
		else {
			info["error_message"s] = "not found"s;
		}
		return info;

	}


	json::Dict StopOutputInfo(transport_catalogue::request_queue::RequestQueue::QueryResult& element, std::string& map) {
		json::Dict info;
		json::Array buses;

		info["request_id"s] = element.id;
		info["map"s] = map;
		
		
		return info;

	}




	
	std::ostream& operator<<(std::ostream& os, std::vector<transport_catalogue::request_queue::RequestQueue::QueryResult>& query_array) {
		if (query_array.empty()) {

			return os;
		}

		json::Array root;




		//int req_num = 0;
		for (auto element : query_array) {
			//req_num++;
			switch (element.query_type_) {

			case RequestQueue::BUS_INFO:


				root.emplace_back(BusInfoOutput(element));



				break;

			case RequestQueue::STOP_INFO:

				root.emplace_back(StopOutputInfo(element));

				break;


			case RequestQueue::MAP_INFO:
			{
				svg::Document doc;
				std::vector <RequestQueue::All_Routes::stop> all_stops;
				size_t color_num = 0;
				for (auto bus : element.all_routes.buses) {

					// рисуем маршруты
					svg::Polyline polyline;
					polyline.SetStrokeColor(element.map_set.color_palette[color_num]);
					polyline.SetFillColor(svg::NoneColor);
					polyline.SetStrokeWidth(element.map_set.line_with);
					polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND); 
					polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
					for (auto stop : bus.stops) {
						const svg::Point coord = element.all_routes.sphere_coord[0]( stop.coordinates_);
						polyline.AddPoint(coord);
						if (std::find(all_stops.begin(), all_stops.end(), stop) == all_stops.end()) {
							all_stops.push_back(stop);
						}
					}
					doc.Add(polyline);
					if ((color_num + 1) < element.map_set.color_palette.size()) {
						color_num++;
					}
					else {
						color_num = 0;
					}


				}

				color_num = 0;// заново считаем те же цвета для остановок

				// отрисовка названий маршрутов
				for (auto bus : element.all_routes.buses) {
					auto first_stop = bus.stops[0];
					
					svg::Text first_stop_text;
					svg::Text first_stop_undertext;

					first_stop_text.SetData(bus.name);
					first_stop_undertext.SetData(bus.name);

					first_stop_undertext.SetFillColor(element.map_set.underlayer_color);
					first_stop_text.SetFillColor(element.map_set.color_palette[color_num]);

					first_stop_undertext.SetStrokeColor(element.map_set.underlayer_color);
					first_stop_undertext.SetStrokeWidth(element.map_set.underlayer_width);
					first_stop_undertext.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
					first_stop_undertext.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
					
					first_stop_text.SetFontSize(element.map_set.bus_label_font_size);
					first_stop_undertext.SetFontSize(element.map_set.bus_label_font_size);

					first_stop_text.SetFontFamily("Verdana");
					first_stop_undertext.SetFontFamily("Verdana");

					first_stop_text.SetFontWeight("bold");
					first_stop_undertext.SetFontWeight("bold");

					svg::Point coord1 = element.all_routes.sphere_coord[0](first_stop.coordinates_);
					first_stop_text.SetPosition(coord1);
					first_stop_undertext.SetPosition(coord1);
					
					coord1.x = element.map_set.bus_label_offset.first;
					coord1.y = element.map_set.bus_label_offset.second;
					first_stop_text.SetOffset(coord1);
					first_stop_undertext.SetOffset(coord1);

					doc.Add(first_stop_undertext);
					doc.Add(first_stop_text);
					
					

					auto last_stop = bus.stops[bus.stops.size() / 2]; // так как маршрут кольцевой - последняя остановка в середине
					if (!bus.is_round && !(first_stop==last_stop) ) {
						
						svg::Text last_stop_text;
						svg::Text last_stop_undertext;

						last_stop_text.SetData(bus.name);
						last_stop_undertext.SetData(bus.name);


						last_stop_text.SetFillColor(element.map_set.color_palette[color_num]);
						last_stop_undertext.SetFillColor(element.map_set.underlayer_color);


						last_stop_undertext.SetStrokeColor(element.map_set.underlayer_color);
						last_stop_undertext.SetStrokeWidth(element.map_set.underlayer_width);
						last_stop_undertext.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
						last_stop_undertext.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

						last_stop_text.SetFontSize(element.map_set.bus_label_font_size);
						last_stop_undertext.SetFontSize(element.map_set.bus_label_font_size);

						last_stop_text.SetFontFamily("Verdana");
						last_stop_undertext.SetFontFamily("Verdana");

						last_stop_text.SetFontWeight("bold");
						last_stop_undertext.SetFontWeight("bold");

						svg::Point coord1 = element.all_routes.sphere_coord[0](last_stop.coordinates_);
						last_stop_text.SetPosition(coord1);
						last_stop_undertext.SetPosition(coord1);

						coord1.x = element.map_set.bus_label_offset.first;
						coord1.y = element.map_set.bus_label_offset.second;
						last_stop_text.SetOffset(coord1);
						last_stop_undertext.SetOffset(coord1);

						doc.Add(last_stop_undertext);
						doc.Add(last_stop_text);
						
					}


					if ((color_num + 1) < element.map_set.color_palette.size()) {
						color_num++;
					}
					else {
						color_num = 0;
					}
				}

				// выводим все остановки
				{
					std::sort(all_stops.begin(), all_stops.end(), [](RequestQueue::All_Routes::stop lhs, RequestQueue::All_Routes::stop rhs)
						{
							return lhs.name < rhs.name;
						});
					
					for (auto stop : all_stops) {
						svg::Circle stop_circle;
						svg::Point coord = element.all_routes.sphere_coord[0](stop.coordinates_);
						stop_circle.SetCenter(coord);
						stop_circle.SetRadius(element.map_set.stop_radius);
						stop_circle.SetFillColor("white");
						doc.Add(stop_circle);
					}



				}

				// выводим названия остановок
				{
					for (auto stop : all_stops) {
						svg::Text stop_text;
						svg::Text stop_undertext;

						stop_text.SetData(stop.name);
						stop_undertext.SetData(stop.name);

						stop_undertext.SetFillColor(element.map_set.underlayer_color);
						stop_text.SetFillColor("black");

						stop_undertext.SetStrokeColor(element.map_set.underlayer_color);
						stop_undertext.SetStrokeWidth(element.map_set.underlayer_width);
						stop_undertext.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
						stop_undertext.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

						stop_text.SetFontSize(element.map_set.stop_label_font_size);
						stop_undertext.SetFontSize(element.map_set.stop_label_font_size);

						stop_text.SetFontFamily("Verdana");
						stop_undertext.SetFontFamily("Verdana");

						svg::Point coord1 = element.all_routes.sphere_coord[0](stop.coordinates_);
						stop_text.SetPosition(coord1);
						stop_undertext.SetPosition(coord1);

						coord1.x = element.map_set.stop_label_offset.first;
						coord1.y = element.map_set.stop_label_offset.second;
						stop_text.SetOffset(coord1);
						stop_undertext.SetOffset(coord1);

						doc.Add(stop_undertext);
						doc.Add(stop_text);
					}
				}

				
				std::ostringstream temp_stream;
				doc.Render(temp_stream);
				
				root.emplace_back(StopOutputInfo(element, temp_stream.str()));

			}
			break;

			default:

				break;

			}

		}
		if (!root.empty()) {
			json::Print(json::Document{ root }, os);
		}
	
		return os;
		}

	

}//namespace transport_catalogue_output_json



namespace transport_catalogue_input_json {

	using namespace std::literals;
	
		RequestQueue::Query AddStop(json::Dict& req) {
			RequestQueue::Query q;
			
			q.name_ = req.at("name").AsString();
			q.coordinates_.lat = req.at("latitude").AsDouble();
			q.coordinates_.lng = req.at("longitude").AsDouble();
			json::Dict json_distances = req.at("road_distances").AsMap();
			for( auto distance : json_distances ){//цикл по списку дистанций до других остановок
				q.distances[std::make_pair(q.name_, distance.first)] = distance.second.AsInt();
				}
			q.query_type_ = RequestQueue::ADD_STOP;
			
			return q;
		}


		RequestQueue::Query AddBus(json::Dict& req) {
			RequestQueue::Query q;
			q.name_ = req.at("name").AsString();
			if (req.at("is_roundtrip").AsBool()) {
				q.query_type_ = RequestQueue::ADD_RING_BUS;
			}
			else {
				q.query_type_ = RequestQueue::ADD_REGULAR_BUS;
			}
			json::Array json_stops = req.at("stops").AsArray();

			for (auto stop : json_stops) {
				 // цикл по списку остановок маршрута
				q.stops.push_back(stop.AsString());
			}
			return q;
		}



		RequestQueue::Query BusInfo(json::Dict& req) {
			RequestQueue::Query q;
			
			q.name_ = req.at("name").AsString();//имя автобуса
			q.query_type_ = RequestQueue::BUS_INFO;
			q.id = req.at("id").AsInt();
			return q;
		}


		RequestQueue::Query StopInfo(json::Dict& req) {
			RequestQueue::Query q;
			q.name_ = req.at("name").AsString();//имя остановки
			q.query_type_ = RequestQueue::STOP_INFO;
			q.id = req.at("id").AsInt();

			return q;
		}


		RequestQueue::Query MapInfo(json::Dict& req) {
			RequestQueue::Query q;
			
			q.query_type_ = RequestQueue::MAP_INFO;
			q.id = req.at("id").AsInt();

			return q;
		}



		void Load_query(const json::Document &doc, 
			std::vector<transport_catalogue::request_queue::RequestQueue::Query>& query_array) {
			


			const json::Node global = doc.GetRoot();
			const json::Dict requests = global.AsMap();
			// запросы на ввод информации
			json::Node base_request = requests.at("base_requests");
			for (auto json_req : base_request.AsArray()) {
				RequestQueue::Query q;
				json::Dict req = json_req.AsMap();
				std::string command = req.at("type").AsString();
				if (command == "Stop") { //добавляем остановку
					q = AddStop(req);

				};

				if (command == "Bus") { // добавляем австобус

					q = AddBus(req);

				}
				query_array.push_back(q);
			}



			// запросы на вывод информации

			json::Node stat_request = requests.at("stat_requests");
			for (auto json_req : stat_request.AsArray()) {
				RequestQueue::Query q;
				json::Dict req = json_req.AsMap();
				std::string command = req.at("type").AsString();

				if (command == "Bus") { //делаем запрос на вывод информации по автобусу

					q = BusInfo(req);

				}
				if (command == "Stop") { //делаем запрос на вывод информации по оствановке
					q = StopInfo(req);
				}

				if (command == "Map") { //делаем запрос на вывод информации по оствановке
					q = MapInfo(req);
				}

				query_array.push_back(q);
			}

			// читаем настройки рендеринга
			{
				json::Node render_settings = requests.at("render_settings");
				json::Dict json_req = render_settings.AsMap();
				RequestQueue::Query q;

				q.query_type_ = RequestQueue::MAP_SETTINGS;
				q.map_set.width = json_req.at("width").AsDouble();
				q.map_set.height = json_req.at("height").AsDouble();
				q.map_set.padding = json_req.at("padding").AsDouble();
				q.map_set.line_with = json_req.at("line_width").AsDouble();
				q.map_set.stop_radius = json_req.at("stop_radius").AsDouble();
				q.map_set.bus_label_font_size = json_req.at("bus_label_font_size").AsInt();
				q.map_set.bus_label_offset.first = json_req.at("bus_label_offset").AsArray()[0].AsDouble();
				q.map_set.bus_label_offset.second = json_req.at("bus_label_offset").AsArray()[1].AsDouble();
				q.map_set.stop_label_font_size = json_req.at("stop_label_font_size").AsInt();
				q.map_set.stop_label_offset.first = json_req.at("stop_label_offset").AsArray()[0].AsDouble();
				q.map_set.stop_label_offset.second = json_req.at("stop_label_offset").AsArray()[1].AsDouble();
				q.map_set.underlayer_width = json_req.at("underlayer_width").AsDouble();
				if (json_req.at("underlayer_color").IsString()) {
					q.map_set.underlayer_color = json_req.at("underlayer_color").AsString();
				}
				if (json_req.at("underlayer_color").IsArray()) {
					if (json_req.at("underlayer_color").AsArray().size() == 3) { //формат RGB
						q.map_set.underlayer_color = svg::Rgb{
								static_cast<uint8_t>(json_req.at("underlayer_color").AsArray()[0].AsInt()),
								static_cast<uint8_t>(json_req.at("underlayer_color").AsArray()[1].AsInt()),
								static_cast<uint8_t>(json_req.at("underlayer_color").AsArray()[2].AsInt())
						};
					}
					else // формат RGBa
					{
						q.map_set.underlayer_color = svg::Rgba{
								static_cast<uint8_t>(json_req.at("underlayer_color").AsArray()[0].AsInt()),
								static_cast<uint8_t>(json_req.at("underlayer_color").AsArray()[1].AsInt()),
								static_cast<uint8_t>(json_req.at("underlayer_color").AsArray()[2].AsInt()),
								json_req.at("underlayer_color").AsArray()[3].AsDouble()
						};
					}
				}



				json::Array colors = json_req.at("color_palette").AsArray();
				for (auto color : colors) {

					if (color.IsString()) {
						q.map_set.color_palette.push_back(color.AsString());
					}
					if (color.IsArray()) {
						if (color.AsArray().size() == 3) { //формат RGB
							q.map_set.color_palette.push_back(svg::Rgb{
								static_cast<uint8_t>(color.AsArray()[0].AsInt()),
								static_cast<uint8_t>(color.AsArray()[1].AsInt()),
								static_cast<uint8_t>(color.AsArray()[2].AsInt()),
								});
						}
						else // формат RGBa
						{
							q.map_set.color_palette.push_back(svg::Rgba{
								static_cast<uint8_t>(color.AsArray()[0].AsInt()),
								static_cast<uint8_t>(color.AsArray()[1].AsInt()),
								static_cast<uint8_t>(color.AsArray()[2].AsInt()),
								color.AsArray()[3].AsDouble()
								});
						}
					}
				}

				query_array.push_back(q);

			}
		
			
		}

		std::istream& operator>>(std::istream& is, std::vector<transport_catalogue::request_queue::RequestQueue::Query>& query_array) {
			
			Load_query(json::Load(is), query_array);
			return is;
		}




}// name space transport_catalogue_input

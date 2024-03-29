#include "json_reader.h"
#include <cassert>
#include "svg.h"
#include "map_renderer.h"
#include <sstream>
#include "request_handler.h"
#include "map_renderer.h"
#include "json_builder.h"


using namespace transport_catalogue;
using namespace request_queue;
using namespace transport;

namespace transport_catalogue_output_json {
	using namespace std::literals;


	json::Dict BusInfoOutput( request_queue::RequestQueue::QueryResult& element) {
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



	json::Dict StopOutputInfo(request_queue::RequestQueue::QueryResult& element) {
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

	json::Dict RouteInfo(request_queue::RequestQueue::QueryResult& element) {
		json::Dict info;
		json::Array routes;

		info["request_id"s] = element.id;
		if (element.route_info.empty()) {
			info["error_message"s] = "not found"s;
			return info;
		}

		info["total_time"s] = element.route_info.front().total_time;

		if (element.route_info.front().total_time != 0) {
			for (auto step : element.route_info) {
				json::Dict step_info;
				if (step.activity == RequestQueue::WAITING) {
					step_info["type"s] = "Wait"s;
					step_info["stop_name"s] = static_cast<string>(step.name);
					step_info["time"s] = step.time;
				}
				if (step.activity == RequestQueue::BUS_MOVING) {
					step_info["type"s] = "Bus"s;
					step_info["bus"s] = static_cast<string>(step.name);
					step_info["span_count"s] = step.span_count;
					step_info["time"s] = step.time;
				}

				routes.push_back(step_info);
			}
		}


		info["items"] = routes;

		return info;

	}



	json::Dict StopOutputInfo(request_queue::RequestQueue::QueryResult& element, std::string map) {
		json::Dict info;
		

		info["request_id"s] = element.id;
		info["map"s] = map;
		
		
		return info;

	}




	
	std::ostream& operator<<(std::ostream& os, std::vector<request_queue::RequestQueue::QueryResult>& query_array) {
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
				
				
					root.emplace_back(StopOutputInfo(element, element.map_data.RenderMap()));

				
				break;


				case RequestQueue::ROUTE_INFO:


					root.emplace_back(RouteInfo(element));


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
	using namespace request_queue;
	
		RequestQueue::Query AddStop(json::Dict& req) {
			RequestQueue::Query q;
			
			q.name_ = req.at("name").AsString();
			q.coordinates_.lat = req.at("latitude").AsDouble();
			q.coordinates_.lng = req.at("longitude").AsDouble();
			json::Dict json_distances = req.at("road_distances").AsDict();
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


		RequestQueue::Query RouteInfo(json::Dict& req) {
			RequestQueue::Query q;
			q.query_type_ = RequestQueue::ROUTE_INFO;
			q.id = req.at("id").AsInt();
			q.stops.push_back(req.at("from").AsString());//имя начальной остановки
			q.stops.push_back(req.at("to").AsString());//имя начальной остановки
			return q;
		}

		RequestQueue::Query MapInfo(json::Dict& req) {
			RequestQueue::Query q;
			
			q.query_type_ = RequestQueue::MAP_INFO;
			q.id = req.at("id").AsInt();

			return q;
		}



		void LoadQuery(const json::Document &doc, 
			std::vector<request_queue::RequestQueue::Query>& query_array, bool make_base_mode) {
			


			const json::Node global = doc.GetRoot();
			const json::Dict requests = global.AsDict();
			// запросы на ввод информации

			if (make_base_mode) {
				json::Node base_request = requests.at("base_requests");
				for (auto json_req : base_request.AsArray()) {
					RequestQueue::Query q;
					json::Dict req = json_req.AsDict();
					std::string command = req.at("type").AsString();
					if (command == "Stop") { //добавляем остановку
						q = AddStop(req);

					};

					if (command == "Bus") { // добавляем австобус

						q = AddBus(req);

					}
					query_array.push_back(q);
				}


				// Читаем настройки роутинга.
				json::Node route_request = requests.at("routing_settings");
				{
					RequestQueue::Query q;
					json::Dict json_req = route_request.AsDict();
					q.query_type_ = RequestQueue::ROUTING_SETTINGS;

					q.route_set.bus_wait_time = json_req.at("bus_wait_time").AsInt();
					q.route_set.bus_velocity = json_req.at("bus_velocity").AsDouble();

					query_array.push_back(q);
				}
				
			}
			

			json::Node serialization_request = requests.at("serialization_settings");
			{
				RequestQueue::Query q;
				json::Dict json_req = serialization_request.AsDict();
				if (make_base_mode) {
					q.query_type_ = RequestQueue::SERIALIZATION_SETTINGS_OUTPUT;
				}
				else {
					q.query_type_ = RequestQueue::SERIALIZATION_SETTINGS_INPUT;
				}
				q.name_ = json_req.at("file").AsString();
				

				query_array.push_back(q);
			}


			// запросы на вывод информации
			if (!make_base_mode) {
				json::Node stat_request = requests.at("stat_requests");
				for (auto json_req : stat_request.AsArray()) {
					RequestQueue::Query q;
					json::Dict req = json_req.AsDict();
					std::string command = req.at("type").AsString();

					if (command == "Bus") { //делаем запрос на вывод информации по автобусу

						q = BusInfo(req);

					}
					if (command == "Stop") { //делаем запрос на вывод информации по оствановке
						q = StopInfo(req);
					}

					if (command == "Map") { //делаем запрос на вывод информации по рисованию карты маршрутов
						q = MapInfo(req);
					}

					if (command == "Route") { //делаем запрос на вывод информации по составлению маршрута от одной остановки до другой
						q = RouteInfo(req);
					}

					query_array.push_back(q);
				}
			}

			if (make_base_mode) {
			// читаем настройки рендеринга
				{
					json::Node render_settings = requests.at("render_settings");
					json::Dict json_req = render_settings.AsDict();
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
						q.map_set.underlayer_color_diff.color_type = map_render::TypeColor::STRING;
						q.map_set.underlayer_color_diff.color_string = json_req.at("underlayer_color").AsString();;
					}
					if (json_req.at("underlayer_color").IsArray()) {
						if (json_req.at("underlayer_color").AsArray().size() == 3) { //формат RGB
							q.map_set.underlayer_color = svg::Rgb{
									static_cast<uint8_t>(json_req.at("underlayer_color").AsArray()[0].AsInt()),
									static_cast<uint8_t>(json_req.at("underlayer_color").AsArray()[1].AsInt()),
									static_cast<uint8_t>(json_req.at("underlayer_color").AsArray()[2].AsInt())
							};
							q.map_set.underlayer_color_diff.color_type = map_render::TypeColor::RGB;
							q.map_set.underlayer_color_diff.color_r = static_cast<uint8_t>(json_req.at("underlayer_color").AsArray()[0].AsInt());
							q.map_set.underlayer_color_diff.color_g = static_cast<uint8_t>(json_req.at("underlayer_color").AsArray()[1].AsInt());
							q.map_set.underlayer_color_diff.color_b = static_cast<uint8_t>(json_req.at("underlayer_color").AsArray()[2].AsInt());
						}
						else // формат RGBa
						{
							q.map_set.underlayer_color = svg::Rgba{
									static_cast<uint8_t>(json_req.at("underlayer_color").AsArray()[0].AsInt()),
									static_cast<uint8_t>(json_req.at("underlayer_color").AsArray()[1].AsInt()),
									static_cast<uint8_t>(json_req.at("underlayer_color").AsArray()[2].AsInt()),
									json_req.at("underlayer_color").AsArray()[3].AsDouble()
							};
							q.map_set.underlayer_color_diff.color_type = map_render::TypeColor::RGBA;
							q.map_set.underlayer_color_diff.color_r = static_cast<uint8_t>(json_req.at("underlayer_color").AsArray()[0].AsInt());
							q.map_set.underlayer_color_diff.color_g = static_cast<uint8_t>(json_req.at("underlayer_color").AsArray()[1].AsInt());
							q.map_set.underlayer_color_diff.color_b = static_cast<uint8_t>(json_req.at("underlayer_color").AsArray()[2].AsInt());
							q.map_set.underlayer_color_diff.color_a = json_req.at("underlayer_color").AsArray()[3].AsDouble();
						}
					}



					json::Array colors = json_req.at("color_palette").AsArray();
					for (auto color : colors) {

						if (color.IsString()) {
							q.map_set.color_palette.push_back(color.AsString());
							map_render::Diff_color cd;
							cd.color_type = map_render::TypeColor::STRING;
							cd.color_string = color.AsString();
							q.map_set.color_palette_diff.push_back(cd);
						}
						if (color.IsArray()) {
							if (color.AsArray().size() == 3) { //формат RGB
								q.map_set.color_palette.push_back(svg::Rgb{
									static_cast<uint8_t>(color.AsArray()[0].AsInt()),
									static_cast<uint8_t>(color.AsArray()[1].AsInt()),
									static_cast<uint8_t>(color.AsArray()[2].AsInt()),
									});
								map_render::Diff_color cd;
								cd.color_type = map_render::TypeColor::RGB;
								cd.color_r = color.AsArray()[0].AsInt();
								cd.color_g = color.AsArray()[1].AsInt();
								cd.color_b = color.AsArray()[2].AsInt();
								q.map_set.color_palette_diff.push_back(cd);

							}
							else // формат RGBa
							{
								q.map_set.color_palette.push_back(svg::Rgba{
									static_cast<uint8_t>(color.AsArray()[0].AsInt()),
									static_cast<uint8_t>(color.AsArray()[1].AsInt()),
									static_cast<uint8_t>(color.AsArray()[2].AsInt()),
									color.AsArray()[3].AsDouble()
									});
								map_render::Diff_color cd;
								cd.color_type = map_render::TypeColor::RGBA;
								cd.color_r = color.AsArray()[0].AsInt();
								cd.color_g = color.AsArray()[1].AsInt();
								cd.color_b = color.AsArray()[2].AsInt();
								cd.color_a = color.AsArray()[3].AsDouble();
								q.map_set.color_palette_diff.push_back(cd);

							}
						}
					}

					query_array.push_back(q);
				}
			}
		
			
		}

		std::istream& operator>>(std::istream& is, std::vector<request_queue::RequestQueue::Query>& query_array) {
			
			LoadQuery(json::Load(is), query_array);
			return is;
		}




}// name space transport_catalogue_input

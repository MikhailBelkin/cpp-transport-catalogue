#include "request_handler.h"
#include "router.h"
#include "graph.h"
#include "transport_catalogue.pb.h"
#include <string>

#include <fstream>

using namespace request_queue;
using namespace transport_catalogue;
using namespace transport;
//using namespace map_render;




std::vector<RequestQueue::RouteInfo> RequestQueue::BuildRoute(const RequestQueue::Query& q) {
	std::vector<RequestQueue::RouteInfo>  result;
	
	if (!tr_) {
		tr_ = std::make_unique<transport_router::TransportRouter>(route_set_.bus_wait_time,
			route_set_.bus_velocity,
			*tc_);
		
	}
	
	auto build_result = tr_->BuildRoute(tc_->FindStop(q.stops[0])->GetId(), tc_->FindStop(q.stops[1])->GetId());
	if (build_result.has_value()) {
		graph::Router<double>::RouteInfo r = build_result.value();
		bool is_first = true;
		RequestQueue::RouteInfo move;
		if (r.edges.size() == 0) {
			// путь нулевой - и так находимся в точке отправления
			RequestQueue::RouteInfo element;
			element.total_time = r.weight;
			result.push_back(element);
			return result;
		}
		for (auto edge : r.edges) {
			RequestQueue::RouteInfo element;
			if (is_first) {
				element.total_time = r.weight; // Total_time пишем просто в первый элемент	
				is_first = false;
			}
			graph::Edge e = tr_->GetEdge(edge);
			//выделяем назад остановку с временем ожидания
			element.name = tc_->GetStop(e.from)->GetName();
			element.time = route_set_.bus_wait_time;
			element.span_count = 0;
			element.activity = ActivityType::WAITING;
			result.push_back(element);
			// выделяем пусть автобуса
			element.name = tc_->GetBus(e.transport_id)->GetName();
			element.time = e.weight-route_set_.bus_wait_time;
			element.span_count = e.span_count;
			element.activity = ActivityType::BUS_MOVING;
			result.push_back(element);
			
		}
	}
	return result;
}

void RequestQueue::Deserialize_TC(tc_serialization::TransportCatalogue& tcs) {
	for (auto stop : tcs.stops()) {
		Coordinates coord;
		coord.lat = stop.mutable_place()->lat();
		coord.lng = stop.mutable_place()->lng();
		Stop new_stop(stop.name(), coord);
		new_stop.SetId(stop.id());
		tc_->AddStop(new_stop);
	}
	for (auto bus : tcs.buses()) {
		std::vector<const Stop*> new_bus_stops(bus.mutable_track()->size());
		for (int i = 0; i < bus.mutable_track()->size(); i++) {
			new_bus_stops[i] = tc_->FindStop(bus.mutable_track(i)->name());
		}
		Bus new_bus(bus.name(), new_bus_stops);
		new_bus.SetId(bus.id());

		if (bus.is_round()) {
			new_bus.SetRound();
		}

		tc_->AddBus(new_bus);

	}
	for (auto item : tcs.dist_list()) {
		auto stop1 = item.stop1();
		auto stop2 = item.stop2();
		tc_->SetDistances(tc_->FindStop(stop1), tc_->FindStop(stop2), item.dist());
	}



}



void RequestQueue::Deserialize_Map(tc_serialization::TransportCatalogue& tcs) {

	map_render::MapRenderSettings map_set;
	/// десериализация настроек рендеринга
	map_set.width = tcs.mutable_map_settings()->width();
	map_set.height = tcs.mutable_map_settings()->height();
	map_set.padding = tcs.mutable_map_settings()->padding();
	map_set.line_with = tcs.mutable_map_settings()->line_with();
	map_set.stop_radius = tcs.mutable_map_settings()->stop_radius();
	map_set.bus_label_font_size = tcs.mutable_map_settings()->bus_label_font_size();
	map_set.bus_label_offset.first = tcs.mutable_map_settings()->bus_label_offset_first();
	map_set.bus_label_offset.second = tcs.mutable_map_settings()->bus_label_offset_second();
	map_set.stop_label_font_size = tcs.mutable_map_settings()->stop_label_font_size();
	map_set.stop_label_offset.first = tcs.mutable_map_settings()->stop_label_offset_first();
	map_set.stop_label_offset.second = tcs.mutable_map_settings()->stop_label_offset_second();

	tc_serialization::ColorVariant c;

	map_set.underlayer_color_diff.color_type = static_cast<map_render::TypeColor>(tcs.mutable_map_settings()->mutable_underlayer_color()->type());


	if (map_set.underlayer_color_diff.color_type == map_render::TypeColor::STRING) {
		map_set.underlayer_color = tcs.mutable_map_settings()->mutable_underlayer_color()->color();

	}
	if (map_set.underlayer_color_diff.color_type == map_render::TypeColor::RGB) {
		map_set.underlayer_color = svg::Rgb{
			tcs.mutable_map_settings()->mutable_underlayer_color()->red(),
			tcs.mutable_map_settings()->mutable_underlayer_color()->green(),
			tcs.mutable_map_settings()->mutable_underlayer_color()->blue()
		};
	}
	if (map_set.underlayer_color_diff.color_type == map_render::TypeColor::RGBA) {

		map_set.underlayer_color = svg::Rgba{
			tcs.mutable_map_settings()->mutable_underlayer_color()->red(),
			tcs.mutable_map_settings()->mutable_underlayer_color()->green(),
			tcs.mutable_map_settings()->mutable_underlayer_color()->blue(),
			tcs.mutable_map_settings()->mutable_underlayer_color()->opacity()
		};

	}

	map_set.underlayer_width = tcs.mutable_map_settings()->underlayer_width();

	map_set.color_palette.resize(tcs.mutable_map_settings()->color_palette_size());
	for (int i = 0; i < map_set.color_palette.size(); i++) {

		auto color_type = static_cast<map_render::TypeColor>(tcs.mutable_map_settings()->color_palette(i).type());

		if (color_type == map_render::TypeColor::STRING) {
			map_set.color_palette[i] = tcs.mutable_map_settings()->color_palette(i).color();

		}
		if (color_type == map_render::TypeColor::RGB) {
			map_set.color_palette[i] = svg::Rgb{
				tcs.mutable_map_settings()->color_palette(i).red(),
				tcs.mutable_map_settings()->color_palette(i).green(),
				tcs.mutable_map_settings()->color_palette(i).blue()
			};
		}
		if (color_type == map_render::TypeColor::RGBA) {
			map_set.color_palette[i] = svg::Rgba{
				tcs.mutable_map_settings()->color_palette(i).red(),
				tcs.mutable_map_settings()->color_palette(i).green(),
				tcs.mutable_map_settings()->color_palette(i).blue(),
				tcs.mutable_map_settings()->color_palette(i).opacity()
			};

		}

	}
	// render_map
	map_data_.SetMapSettings(map_set);
	map_data_.SetAllroutes(GetAllRoutes());

}


void RequestQueue::Deserialize_Router(tc_serialization::TransportCatalogue& tcs) {
	route_set_.bus_velocity = tcs.mutable_route_info()->bus_velocity();
	route_set_.bus_wait_time = tcs.mutable_route_info()->bus_wait_time();
	route_set_.bus_leaving_time = tcs.mutable_route_info()->bus_leaving_time();


	std::vector<graph::Edge<double>>  edges;
	for (int i = 0; i < tcs.mutable_all_graphs()->size(); i++) {
		graph::Edge<double> item;
		item.from = tcs.mutable_all_graphs(i)->from();
		item.to = tcs.mutable_all_graphs(i)->to();
		item.weight = tcs.mutable_all_graphs(i)->weight();
		item.transport_id = tcs.mutable_all_graphs(i)->transport_id();
		item.span_count = tcs.mutable_all_graphs(i)->span_count();

		edges.push_back(item);
	}

	tr_ = std::make_unique<transport_router::TransportRouter>(route_set_.bus_wait_time,
		route_set_.bus_velocity, edges,
		*tc_);

}



void RequestQueue::Serialize_Router(std::string db_file_name, Query q) {
	tc_serialization::TransportCatalogue tcs;
	tc_serialization::RouteInfo ri;
	ri.set_bus_leaving_time(q.route_set.bus_leaving_time);
	ri.set_bus_velocity(q.route_set.bus_velocity);
	ri.set_bus_wait_time(q.route_set.bus_wait_time);

	*tcs.mutable_route_info() = ri;


	if (!tr_) {
		tr_ = std::make_unique<transport_router::TransportRouter>(route_set_.bus_wait_time,
			route_set_.bus_velocity,
			*tc_);

	}
	auto edges = tr_.get()->GetAllGraph();
	int edge_count = 0;
	for (auto item : edges) {
		tc_serialization::Edge_Route e;
		e.set_from(item.from);
		e.set_to(item.to);
		e.set_weight(item.weight);
		e.set_transport_id(item.transport_id);
		e.set_span_count(item.span_count);
		tcs.add_all_graphs();
		*tcs.mutable_all_graphs(edge_count++) = e;
	}

	std::ofstream out(db_file_name, std::ios::binary | std::ios::app);

	tcs.SerializeToOstream(&out);



}


void RequestQueue::Serialize_Map(std::string db_file_name, Query q) {
	tc_serialization::TransportCatalogue tcs;
	tc_serialization::MapRenderSettings mrs;
	mrs.set_width(q.map_set.width);
	mrs.set_height(q.map_set.height);
	mrs.set_padding(q.map_set.padding);
	mrs.set_line_with(q.map_set.line_with);
	mrs.set_stop_radius(q.map_set.stop_radius);
	mrs.set_bus_label_font_size(q.map_set.bus_label_font_size);
	mrs.set_bus_label_offset_first(q.map_set.bus_label_offset.first);
	mrs.set_bus_label_offset_second(q.map_set.bus_label_offset.second);
	mrs.set_stop_label_font_size(q.map_set.stop_label_font_size);
	mrs.set_stop_label_offset_first(q.map_set.stop_label_offset.first);
	mrs.set_stop_label_offset_second(q.map_set.stop_label_offset.second);
	tc_serialization::ColorVariant c;

	c.set_type(q.map_set.underlayer_color_diff.color_type);
	if (q.map_set.underlayer_color_diff.color_type == map_render::TypeColor::STRING) {
		c.set_color(q.map_set.underlayer_color_diff.color_string);
	}
	if (q.map_set.underlayer_color_diff.color_type == map_render::TypeColor::RGB) {
		c.set_red(q.map_set.underlayer_color_diff.color_r);
		c.set_green(q.map_set.underlayer_color_diff.color_g);
		c.set_blue(q.map_set.underlayer_color_diff.color_b);
	}
	if (q.map_set.underlayer_color_diff.color_type == map_render::TypeColor::RGBA) {
		c.set_red(q.map_set.underlayer_color_diff.color_r);
		c.set_green(q.map_set.underlayer_color_diff.color_g);
		c.set_blue(q.map_set.underlayer_color_diff.color_b);
		c.set_opacity(q.map_set.underlayer_color_diff.color_a);

	}

	*mrs.mutable_underlayer_color() = c;

	mrs.set_underlayer_width(q.map_set.underlayer_width);

	for (int i = 0; i < q.map_set.color_palette_diff.size(); i++) {
		tc_serialization::ColorVariant c;

		c.set_type(q.map_set.color_palette_diff[i].color_type);
		if (q.map_set.color_palette_diff[i].color_type == map_render::TypeColor::STRING) {
			c.set_color(q.map_set.color_palette_diff[i].color_string);
		}
		if (q.map_set.color_palette_diff[i].color_type == map_render::TypeColor::RGB) {
			c.set_red(q.map_set.color_palette_diff[i].color_r);
			c.set_green(q.map_set.color_palette_diff[i].color_g);
			c.set_blue(q.map_set.color_palette_diff[i].color_b);
		}
		if (q.map_set.color_palette_diff[i].color_type == map_render::TypeColor::RGBA) {
			c.set_red(q.map_set.color_palette_diff[i].color_r);
			c.set_green(q.map_set.color_palette_diff[i].color_g);
			c.set_blue(q.map_set.color_palette_diff[i].color_b);
			c.set_opacity(q.map_set.color_palette_diff[i].color_a);

		}

		mrs.add_color_palette();
		*mrs.mutable_color_palette(i) = c;


	}

	*tcs.mutable_map_settings() = mrs;
	std::ofstream out(db_file_name, std::ios::binary | std::ios::app);

	tcs.SerializeToOstream(&out);


}


void RequestQueue::Serialize_TC(std::string db_file_name) {
	tc_serialization::TransportCatalogue tcs;
	for (auto stop : tc_->GetAllStops()) {
		auto new_stop = tcs.add_stops();
		new_stop->set_name(stop.GetName());
		new_stop->set_id(stop.GetId());
		new_stop->mutable_place()->set_lat(stop.GetCoordonates().lat);
		new_stop->mutable_place()->set_lng(stop.GetCoordonates().lng);
	}
	for (auto bus : tc_->GetAllRoutes()) {
		auto new_bus = tcs.add_buses();
		new_bus->set_name(bus.GetName());
		new_bus->set_id(bus.GetId());
		new_bus->set_is_round(bus.isRound());
		auto track = bus.GetBusInfo();
		for (int i = 0; i < bus.GetStopsNum(); i++) {
			new_bus->add_track();
			new_bus->mutable_track(i)->set_name(track[i]->GetName()); // имени остановки достаточно
		}
	}

	auto dist = tc_->GetAllDistances();
	for (auto item : dist) {
		auto d = tcs.add_dist_list();
		d->set_stop1(item.first.first);
		d->set_stop2(item.first.second);
		d->set_dist(item.second);

	}


	std::ofstream out(db_file_name, std::ios::binary);

	tcs.SerializeToOstream(&out);


}


std::vector<RequestQueue::QueryResult> RequestQueue::ProcessQueue() {
	//ADD_STOP,
	//ADD_REGULAR_BUS,
	//	ADD_RING_BUS,
	//	BUS_INFO
	//std::unordered_map<QueryType, std::deque<Query>> requests_;

	std::vector<QueryResult> result;
	std::string db_file_name="";

	if (requests_.count(ADD_STOP) != 0) {
		std::unordered_map<std::pair<std::string, std::string>, int, QDistHasher> dist;
		for (Query q : requests_.at(ADD_STOP)) {

			Stop s(q.name_, q.coordinates_);
			tc_->AddStop(s);

			if (q.distances.size() > 0) {
				dist.merge(q.distances);
			}
		}
		for (auto [stops, distance] : dist) {
			//std::unordered_map<std::pair<std::string, std::string>, int> distances;
			tc_->SetDistances(tc_->FindStop(stops.first),
				tc_->FindStop(stops.second),
				distance);
		}

	}
	if (requests_.count(ADD_RING_BUS) != 0) {
		for (Query q : requests_.at(ADD_RING_BUS)) {
			std::vector<const Stop*> s;
			for (std::string& stop : q.stops) {
				const Stop* s_ptr = tc_->FindStop(stop);
				if (s_ptr) {
					s.push_back(s_ptr);
				}
				else {
					throw  std::invalid_argument("Bus Stop is not existing" + stop);
				}

			}
			Bus b(q.name_, s);
			b.SetRound();
			tc_->AddBus(b);
		}
	}

	if (requests_.count(ADD_REGULAR_BUS) != 0) {
		for (Query q : requests_.at(ADD_REGULAR_BUS)) {
			std::vector<const Stop*> s;
			for (std::string& stop : q.stops) {
				const Stop* s_ptr = tc_->FindStop(stop);
				if (s_ptr) {
					s.push_back(s_ptr);
				}
				else {
					throw  std::invalid_argument("Bus Stop is not existing" + stop);
				}

			}
			// Запоминаем количество остановок для цикла
			int stop_num = s.size();
			// в цикле добавляем те же остановки в обратном направлении начиная с предпоследней
			for (int i = stop_num - 2; i >= 0; i--) {
				s.push_back(s[i]);
			}
			Bus b(q.name_, s);

			tc_->AddBus(b);
		}
	}



	if (requests_.count(SERIALIZATION_SETTINGS_OUTPUT) != 0) {
		for (Query q : requests_.at(SERIALIZATION_SETTINGS_OUTPUT)) {
			if (q.query_type_ == SERIALIZATION_SETTINGS_OUTPUT) {

				db_file_name = q.name_;
	
				Serialize_TC(db_file_name);

				//tcs.PrintDebugString();

			}
		}
	}


	if (requests_.count(MAP_SETTINGS) != 0) {
		for (Query q : requests_.at(MAP_SETTINGS)) {
			if (q.query_type_ == MAP_SETTINGS) {
				QueryResult res;
				res.query_type_ = MAP_SETTINGS;
				map_data_.SetMapSettings(q.map_set);
				map_data_.SetAllroutes(GetAllRoutes());
				if (db_file_name != "") {
					Serialize_Map(db_file_name, q);
				}


				result.push_back(res);
			}
		}
	}

	if (requests_.count(ROUTING_SETTINGS) != 0) {
		for (Query q : requests_.at(ROUTING_SETTINGS)) {
			if (q.query_type_ == ROUTING_SETTINGS) {
				QueryResult res;
				res.query_type_ = ROUTING_SETTINGS;
				route_set_.bus_velocity = q.route_set.bus_velocity;
				route_set_.bus_wait_time = q.route_set.bus_wait_time;
				route_set_.bus_leaving_time = q.route_set.bus_leaving_time;

				if (db_file_name != "") {
					Serialize_Router(db_file_name, q);
				}



				result.push_back(res);
			}
		}
	}




	if (requests_.count(SERIALIZATION_SETTINGS_INPUT) != 0) {
		for (Query q : requests_.at(SERIALIZATION_SETTINGS_INPUT)) {
			if (q.query_type_ == SERIALIZATION_SETTINGS_INPUT) {
				tc_serialization::TransportCatalogue tcs;
				std::ifstream in(q.name_, std::ios::binary);
				tcs.ParseFromIstream(&in);
				
				Deserialize_TC(tcs);
				Deserialize_Map(tcs);

				QueryResult res_route;
				res_route.query_type_ = MAP_SETTINGS;
				result.push_back(res_route);


				// route_map
				QueryResult res_map;
				res_map.query_type_ = ROUTING_SETTINGS;
				result.push_back(res_map);

				Deserialize_Router(tcs);
			}
		}
	}




	if (requests_.count(GET_INFO) != 0) {

		for (Query q : requests_.at(GET_INFO)) {
			if (q.query_type_ == BUS_INFO) {
				QueryResult res;
				res.id = q.id;
				res.name_ = q.name_;
				res.query_type_ = BUS_INFO;
				Coordinates coord;
				coord.lat = 0;
				coord.lng = 0;
				std::string prev_stop;
				if (tc_->FindBus(q.name_) != nullptr) {  //автобуса не существует. остановки не заполняем, длинну маршрута тоже
					res.found = true;
					for (auto stop : tc_->FindBus(q.name_)->GetBusInfo()) {
						res.stops.push_back(stop->GetName());
						if (coord.lat == 0.0 && coord.lng == 0.0) {
							//std::cout<<"0!"<<stop->GetName()<< std::endl;
							coord = stop->GetCoordonates();
							prev_stop = stop->GetName();
						}
						else {

							//std::cout<<coord.lat<<"," <<coord.lng<<" "<< stop->GetName()<< std::endl;
							res.track_lenght += ComputeDistance(coord, stop->GetCoordonates());
							res.track_distance += tc_->GetDistance(
								tc_->FindStop(prev_stop),
								tc_->FindStop(stop->GetName())
							);
							prev_stop = stop->GetName();
							coord = stop->GetCoordonates();
						}

					}
				}
				else {
					res.found = false;
				}
				result.push_back(res);
			}
			if (q.query_type_ == STOP_INFO) {
				QueryResult res;
				res.id = q.id;
				res.name_ = q.name_;
				res.query_type_ = STOP_INFO;
				if (tc_->FindStop(q.name_) != nullptr) {
					res.found = true;
					for (auto bus : tc_->GetBusesForStop(q.name_)) {
						res.buses.push_back(bus.GetName());
					}
				}
				else {
					res.found = false;
				}
				result.push_back(res);
			}
			if (q.query_type_ == MAP_INFO) {
				QueryResult res;
				res.id = q.id;
				res.query_type_ = MAP_INFO;
				res.map_data = map_data_;
				
				result.push_back(res);
			}

			if (q.query_type_ == ROUTE_INFO) {
				QueryResult res;
				res.id = q.id;
				res.query_type_ = ROUTE_INFO;
				

				res.route_info = BuildRoute(q);




				result.push_back(res);
			}



		}

	}





	return result;
}



map_render::MapRender::AllRoutes RequestQueue::GetAllRoutes() {
	map_render::MapRender::AllRoutes result;
	std::vector<Bus> buses;
	buses = tc_->GetAllRoutes();
	std::sort(buses.begin(), buses.end(), [](Bus lhs, Bus rhs)
		{
			return lhs.GetName() < rhs.GetName();
		});
	std::vector<geo::Coordinates> points;
	for (auto bus : buses) {
		map_render::MapRender::AllRoutes::Bus current_bus;
		std::vector<map_render::MapRender::AllRoutes::Stop> current_bus_stops;
		std::vector<const Stop*> stops;
		current_bus.name = bus.GetName();
		current_bus.is_round = bus.isRound();
		stops = bus.GetBusInfo();
		for (const Stop* stop : stops) {
			map_render::MapRender::AllRoutes::Stop stop_for_save;
			stop_for_save.name = stop->GetName();
			stop_for_save.coordinates_ = stop->GetCoordonates();
			points.push_back(stop_for_save.coordinates_);
			current_bus_stops.push_back(stop_for_save);

		}
		current_bus.stops = current_bus_stops;


		result.buses.push_back(current_bus);


	}

	result.sphere_coord.push_back(
		map_render::SphereProjector(
			points.begin(),
			points.end(),
			map_data_.GetMapSettings().width,
			map_data_.GetMapSettings().height,
			map_data_.GetMapSettings().padding
		)
	);


	return result;

}
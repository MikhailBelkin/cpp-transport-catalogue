#include "request_handler.h"
#include "router.h"
#include "graph.h"
#include "transport_catalogue.pb.h"
#include "serialization.h"
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
	
				Serialize_TC(db_file_name, *tc_);

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
					Serialize_Map(db_file_name, q.map_set);
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

					if (!tr_) {
						tr_ = std::make_unique<transport_router::TransportRouter>(route_set_.bus_wait_time,
							route_set_.bus_velocity,
							*tc_);

					}

					Serialize_Router(db_file_name, q.route_set, *tr_);
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
				
				Deserialize_TC(tcs, *tc_);
				
				// render_map
				map_data_.SetMapSettings(Deserialize_Map(tcs));
				map_data_.SetAllroutes(GetAllRoutes());


				QueryResult res_route;
				res_route.query_type_ = MAP_SETTINGS;
				result.push_back(res_route);


				// route_map
				QueryResult res_map;
				res_map.query_type_ = ROUTING_SETTINGS;
				result.push_back(res_map);
				std::vector<graph::Edge<double>> edges;
				route_set_ = Deserialize_Router(tcs, edges);

				tr_ = std::make_unique<transport_router::TransportRouter>(route_set_.bus_wait_time,
					route_set_.bus_velocity, edges,
					*tc_);

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
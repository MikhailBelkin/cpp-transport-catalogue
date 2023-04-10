#include "request_handler.h"

using namespace request_queue;
using namespace transport_catalogue;
using namespace transport;
//using namespace map_render;

std::vector<RequestQueue::QueryResult> RequestQueue::ProcessQueue() {
	//ADD_STOP,
	//ADD_REGULAR_BUS,
	//	ADD_RING_BUS,
	//	BUS_INFO
	//std::unordered_map<QueryType, std::deque<Query>> requests_;

	std::vector<QueryResult> result;

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
			std::vector<Stop*> s;
			for (std::string& stop : q.stops) {
				Stop* s_ptr = tc_->FindStop(stop);
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
			std::vector<Stop*> s;
			for (std::string& stop : q.stops) {
				Stop* s_ptr = tc_->FindStop(stop);
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

	if (requests_.count(MAP_SETTINGS) != 0) {
		for (Query q : requests_.at(MAP_SETTINGS)) {
			if (q.query_type_ == MAP_SETTINGS) {
				QueryResult res;
				res.query_type_ = MAP_SETTINGS;
				map_data_.SetMapSettings(q.map_set);
				map_data_.SetAllroutes( GetAllRoutes() );

				result.push_back(res);
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
		std::vector<Stop*> stops;
		current_bus.name = bus.GetName();
		current_bus.is_round = bus.isRound();
		stops = bus.GetBusInfo();
		for (Stop* stop : stops) {
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
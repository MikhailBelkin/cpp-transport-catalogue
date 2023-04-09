// напишите решение с нуля
// код сохраните в свой git-репозиторий
// напишите решение с нуля
// код сохраните в свой git-репозиторий
#include "transport_catalogue.h"
#include <vector>
#include <stdexcept>
#include <string>
#include <iostream>


using namespace transport_catalogue;
using namespace transport_catalogue::transport;
using namespace request_queue;


void TransportCatalogue::AddStop(const Stop& stop) {
	stops_.push_back(std::move(stop));
	stops_index_[stops_.back().GetName()] = &stops_.back();
}

Stop* TransportCatalogue::FindStop(const std::string& name) const {
	if (stops_index_.count(name) != 0) {
		return stops_index_.at(name);
	}
	else {
		return nullptr;
	}
}

void TransportCatalogue::SetDistances(Stop* stop1, Stop* stop2, int distance) {
	distances_[std::make_pair(stop1, stop2)] = distance;
}

int TransportCatalogue::GetDistance(Stop* stop1, Stop* stop2) {
	if (distances_.count(std::make_pair(stop1, stop2)) != 0) {
		return distances_[std::make_pair(stop1, stop2)];
	}
	else
		if (distances_.count(std::make_pair(stop2, stop1)) != 0) {

			return distances_[std::make_pair(stop2, stop1)];
		}
		else {
			throw std::invalid_argument("Cannot find distanse" + stop1->GetName() + stop2->GetName());
		}
}





//Bus Methods
void TransportCatalogue::AddBus(Bus& bus) {
	buses_.push_back(std::move(bus));
	buses_index_[buses_.back().GetName()] = &buses_.back();

}

Bus* TransportCatalogue::FindBus(const std::string& name) {
	if (buses_index_.count(name) != 0) {
		return buses_index_.at(name);
	}
	else {
		return nullptr;
	}
}

std::vector<Bus> TransportCatalogue::GetBusesForStop(const std::string stop_name) {
	std::vector<Bus> result;
	for (auto bus : buses_) {
		bool yes = false;
		auto stops = bus.GetBusInfo();
		for (auto stop : stops) {
			if (stop->GetName() == stop_name) {
				yes = true;
			}
		}
		if (yes) {
			result.push_back(bus);
		}
	}
	return result;
}

std::vector<Bus> TransportCatalogue::GetAllRoutes() {
	std::vector<Bus> result;

	for (auto bus : buses_) {
		result.push_back(bus);

		
	
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
	if (requests_.count(ADD_RING_BUS ) != 0) {
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
				map_data_.map_set = q.map_set;
				map_data_.all_routes = GetAllRoutes();

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
				res.all_routes = map_data_.all_routes;
				res.map_set = map_data_.map_set;
				result.push_back(res);
			}
			
		}
	
	}

	
		
	

	return result;
}



RequestQueue::All_Routes RequestQueue::GetAllRoutes() {
	RequestQueue::All_Routes result;
	std::vector<Bus> buses;
	buses = tc_->GetAllRoutes();
	std::sort(buses.begin(), buses.end(), [](Bus lhs, Bus rhs)
		{
			return lhs.GetName() < rhs.GetName();
		});
	std::vector<geo::Coordinates> points;
	for (auto bus : buses) {
		RequestQueue::All_Routes::bus current_bus;
		std::vector<RequestQueue::All_Routes::stop> current_bus_stops;
		std::vector<Stop*> stops;
		current_bus.name = bus.GetName();
		current_bus.is_round = bus.isRound();
		stops = bus.GetBusInfo();
		for (Stop* stop:stops ){
			RequestQueue::All_Routes::stop stop_for_save;
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
			map_data_.map_set.width,
			map_data_.map_set.height,
			map_data_.map_set.padding
		)
	);


	return result;

}
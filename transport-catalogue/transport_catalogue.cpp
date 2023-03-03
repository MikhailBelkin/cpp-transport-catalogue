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
			Bus b(q.name_, s);
			tc_->AddBus(b);
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

	if (requests_.count(GET_INFO) != 0) {
		for (Query q : requests_.at(GET_INFO)) {
			if (q.query_type_ == BUS_INFO) {
				QueryResult res;
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

		}
	}

	
		
	

	return result;
}
#pragma once


#include <string>
#include <string_view>
#include <deque>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cstdint>
#include <map>


#include "domain.h"


namespace transport_catalogue {
	using namespace geo;
	using namespace transport;

		



		class TransportCatalogue {
		public:
			TransportCatalogue() {};

//Stops Methods

			void AddStop(const Stop& stop);

			const Stop* FindStop(const std::string& name) const;

			void SetDistances(const Stop* stop1, const Stop* stop2, int distance);

			int GetDistance(const Stop* stop1, const Stop* stop2) const;

			const Stop* GetStop(int id) const {
				return &stops_[id];
			}


			const Bus* GetBus(int id) const {
				return &buses_[id];
			}

			int GetElementsCount() const {
				int count=0;
				for (auto bus : buses_) {
					count += bus.GetStopsNum();
				}
				return count;
			}

			int GetStopsCount() const {
				
				return stops_.size();
			}

//Bus Methods
			void AddBus(Bus& bus);

			const Bus* FindBus(const std::string& name) const;

			std::vector<Bus> GetBusesForStop(const std::string stop_name);
			std::vector<Bus> GetAllRoutes();


		private:
			//stops filds
			std::deque<Stop> stops_;
			std::unordered_map<std::string_view, const Stop*, StopHasher>  stops_index_;
			std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistHasher> distances_;

			//bus fields

			std::deque<Bus> buses_;
			std::unordered_map<std::string, Bus*, BusHasher>  buses_index_;
			
		};



	}


	

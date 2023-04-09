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

			Stop* FindStop(const std::string& name) const;

			void SetDistances(Stop* stop1, Stop* stop2, int distance);

			int GetDistance(Stop* stop1, Stop* stop2);





//Bus Methods
			void AddBus(Bus& bus);

			Bus* FindBus(const std::string& name);

			std::vector<Bus> GetBusesForStop(const std::string stop_name);
			std::vector<Bus> GetAllRoutes();


		private:
			//stops filds
			std::deque<Stop> stops_;
			std::unordered_map<std::string_view, Stop*, StopHasher>  stops_index_;
			std::unordered_map<std::pair<Stop*, Stop*>, int, DistHasher> distances_;

			//bus fields

			std::deque<Bus> buses_;
			std::unordered_map<std::string, Bus*, BusHasher>  buses_index_;
			
		};



	}


	

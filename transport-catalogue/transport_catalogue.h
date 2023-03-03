#pragma once

#include "geo.h"
#include <string>
#include <string_view>
#include <deque>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cstdint>
#include <map>


namespace transport_catalogue {


	namespace transport {

		class Stop {
		public:


			explicit Stop(const std::string& name, const Coordinates& place) :name_(move(name)), place_(place) {

			}

			const std::string& GetName() const {
				return name_;
			}

			void SetName(const std::string& name) {
				name_ = std::move(name);
			}

			const Coordinates GetCoordonates() {
				return place_;
			}
			void SetCoordinates(const Coordinates& place) {
				place_ = std::move(place);
			}

		private:
			std::string name_;
			Coordinates place_;
		};


		class StopHasher {
		public:


			size_t operator()(std::string_view stop_name) const {

				std::hash<std::string_view> stop_hash;
				return stop_hash(stop_name);

			}


		private:

		};


		class DistHasher {
		public:


			size_t operator()(std::pair<Stop*, Stop*> stop_dist) const {

				std::hash<uint64_t> dist_hash;
				uint64_t s1 = uint64_t(stop_dist.first) * 10 + uint64_t(stop_dist.second);

				return dist_hash(s1);

			}


		private:

		};







		class Bus {
		public:
			explicit Bus(const std::string& name, const std::vector<Stop*>& stops) :name_(std::move(name)) {
				for (auto s : stops) {
					track_.push_back(s);
				}

			};

			const std::string& GetName() {
				return name_;
			}

			std::vector<Stop*>& GetBusInfo() {
				return track_;
			}

		private:
			std::string name_;
			std::vector<Stop*> track_;
		};


		class BusHasher {
		public:



			size_t operator()(std::string bus_name) const {


				return bus_hash(bus_name);

			}


		private:
			std::hash<std::string> bus_hash;

		};


		



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


	namespace request_queue {

		class RequestQueue {
		public:

			enum QueryType {
				ADD_STOP,
				ADD_REGULAR_BUS,
				ADD_RING_BUS,
				GET_INFO,
				BUS_INFO,
				STOP_INFO
			};



			class QDistHasher {
			public:


				size_t operator()(std::pair<std::string, std::string> stop_dist) const {

					std::hash<std::string> dist_hash;
					std::string s1 = stop_dist.first + stop_dist.second;

					return dist_hash(s1);

				}


			private:

			};




			struct Query {
				QueryType query_type_;
				std::string name_;
				Coordinates coordinates_;
				std::vector<std::string> stops;
				std::unordered_map<std::pair<std::string, std::string>, int, QDistHasher> distances;
			};


			struct QueryResult {
				QueryType query_type_;
				std::string name_;
				Coordinates coordinates_;
				std::vector<std::string> stops;
				std::vector<std::string> buses;
				double track_lenght = 0;
				double track_distance = 0;
				bool found;
			};

			explicit RequestQueue(transport::TransportCatalogue& tc) {
				tc_ = &tc;
			}

			void AddRequest(Query& q) {
				if (q.query_type_ > GET_INFO) {
					requests_[GET_INFO].push_back(q);
				}
				else {
					requests_[q.query_type_].push_back(q);
				}
			}

			std::vector<QueryResult> ProcessQueue();


		private:

			std::unordered_map<QueryType, std::deque<Query>> requests_;
			transport::TransportCatalogue* tc_;
		};



	}

}
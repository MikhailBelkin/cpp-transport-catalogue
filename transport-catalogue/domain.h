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
#include "geo.h"



namespace transport {

	using namespace geo;

	class Stop {
	public:


		explicit Stop(const std::string& name, const Coordinates& place) :name_(move(name)), place_(place) {
			id_ = stops_count_++;
		}

		const std::string& GetName() const {
			return name_;
		}

		int GetId() const {
			return id_;
		}

		void SetName(const std::string& name) {
			name_ = std::move(name);
		}

		Coordinates GetCoordonates() const {
			return place_;
		}
		void SetCoordinates(const Coordinates& place) {
			place_ = std::move(place);
		}

	private:
		static int stops_count_;
		std::string name_;
		Coordinates place_;
		int id_;
		
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


		size_t operator()(std::pair<const Stop*, const Stop*> stop_dist) const {

			std::hash<uint64_t> dist_hash;
			uint64_t s1 = uint64_t(stop_dist.first) * 10 + uint64_t(stop_dist.second);

			return dist_hash(s1);

		}


	private:

	};







	class Bus {
	public:
		explicit Bus(const std::string& name, const std::vector<const Stop*>& stops) :name_(std::move(name)) {
			for (auto s : stops) {
				track_.push_back(s);
			}
			id_ = bus_count_++;

		};

		const std::string& GetName() const {
			return name_;
		}

		int GetId() const{
			return id_;
		}

		const std::vector<const Stop*>& GetBusInfo() const {
			return track_;
		}
		bool isRound()  const{ return is_round_; }
		void SetRound() { is_round_ = true; }
		int GetStopsNum() {
			return track_.size();
		}

	private:
		static int bus_count_;
		std::string name_;
		std::vector<const Stop*> track_;
		bool is_round_ = false;
		int id_;
	};


	class BusHasher {
	public:



		size_t operator()(std::string bus_name) const {


			return bus_hash(bus_name);

		}


	private:
		std::hash<std::string> bus_hash;


	};



}


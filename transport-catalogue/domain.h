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
		bool isRound() { return is_round_; }
		void SetRound() { is_round_ = true; }

	private:
		std::string name_;
		std::vector<Stop*> track_;
		bool is_round_ = false;
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


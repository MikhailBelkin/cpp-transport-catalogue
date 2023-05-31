#include "transport_catalogue.h"
#include <vector>
#include <stdexcept>
#include <string>
#include <iostream>


using namespace transport_catalogue;

using namespace transport;


void TransportCatalogue::AddStop(const Stop& stop) {
	stops_.push_back(std::move(stop));
	stops_.back().SetId(stops_.size()-1);
	stops_index_[stops_.back().GetName()] = &stops_.back();
}

const Stop* TransportCatalogue::FindStop(const std::string& name) const {
	if (stops_index_.count(name) != 0) {
		return stops_index_.at(name);
	}
	else {
		return nullptr;
	}
}

void TransportCatalogue::SetDistances(const Stop* stop1, const Stop* stop2, int distance) {
	distances_[std::make_pair(stop1, stop2)] = distance;
}

int TransportCatalogue::GetDistance(const Stop* stop1, const Stop* stop2) const{
	if (distances_.count(std::make_pair(stop1, stop2)) != 0) {
		return distances_.at(std::make_pair(stop1, stop2));
	}
	else
		if (distances_.count(std::make_pair(stop2, stop1)) != 0) {

			return distances_.at(std::make_pair(stop2, stop1));
		}
		else {
			throw std::invalid_argument("Cannot find distanse" + stop1->GetName() + stop2->GetName());
		}
}





//Bus Methods
void TransportCatalogue::AddBus(Bus& bus) {
	buses_.push_back(std::move(bus));
	buses_.back().SetId(buses_.size() - 1);
	buses_index_[buses_.back().GetName()] = &buses_.back();

}

const Bus* TransportCatalogue::FindBus(const std::string& name) const{
	if (buses_index_.count(name) != 0) {
		return buses_index_.at(name);
	}
	else {
		return nullptr;
	}
}

std::vector<Bus> TransportCatalogue::GetBusesForStop(const std::string stop_name) const {
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

std::vector<Bus> TransportCatalogue::GetAllRoutes() const {
	std::vector<Bus> result;

	for (auto bus : buses_) {
		result.push_back(bus);

		
	
	}
	return result;



}


std::vector<Stop> TransportCatalogue::GetAllStops() const {
	std::vector<Stop> result;

	for (auto stop : stops_) {

		result.push_back(stop);

	}

	return result;

}

std::map < std::pair<std::string, std::string>, int> TransportCatalogue::GetAllDistances() const {

	std::map < std::pair<std::string, std::string>, int> result;
	for (auto item : this->distances_) {
		result[make_pair(item.first.first->GetName(), item.first.second->GetName())] = item.second;
	}
	return result;

}

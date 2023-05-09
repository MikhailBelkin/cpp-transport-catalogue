#include "transport_router.h"
#include "domain.h"

using namespace transport;

const double METERS_IN_1KM = 1000.0;
const double MINUTS_IN_1HOUR = 60.0;

namespace transport_router {

	void TransportRouter::Fill_Graphs(transport_catalogue::TransportCatalogue &tc) {
		all_graphs_ = std::make_unique<graph::DirectedWeightedGraph<double>>
			(graph::DirectedWeightedGraph<double>(tc.GetStopsCount()));


		for (auto bus : tc.GetAllRoutes()) {
			//std::vector<Stop*> stops;
			auto bus_stops = bus.GetBusInfo();
			for (int stops_from_count = 0; stops_from_count < bus_stops.size(); stops_from_count++) {
				const Stop* stop_start = bus_stops[stops_from_count];

				graph::Edge<double> bus_start_edge;
				bus_start_edge.from = stop_start->GetId();
				bus_start_edge.to = stop_start->GetId();
				bus_start_edge.weight = 0;
				bus_start_edge.transport_id = bus.GetId();
				bus_start_edge.span_count = 0;

				all_graphs_.get()->AddEdge(bus_start_edge);
				int distance = 0;
				for (int stops_to_count = stops_from_count + 1; stops_to_count < bus_stops.size(); stops_to_count++) {
					const Stop* stop_finish = bus_stops[stops_to_count];
					distance += tc.GetDistance(bus_stops[stops_to_count - 1], stop_finish);
					graph::Edge<double> bus_finish_edge;
					bus_finish_edge.from = stop_start->GetId();
					bus_finish_edge.to = stop_finish->GetId();
					bus_finish_edge.weight = bus_wait_time_ + distance / METERS_IN_1KM/ (bus_velocity_ / MINUTS_IN_1HOUR);
					bus_finish_edge.transport_id = bus.GetId();
					bus_finish_edge.span_count = stops_to_count - stops_from_count;
					all_graphs_.get()->AddEdge(bus_finish_edge);
				}

			}
		}
	}

	std::optional<graph::Router<double>::RouteInfo> TransportRouter::BuildRoute(graph::VertexId from, graph::VertexId to) {
		return route_->BuildRoute(from, to);
	}
}
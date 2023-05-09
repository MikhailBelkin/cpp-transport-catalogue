#pragma once
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"
#include <memory>




namespace transport_router {


	class TransportRouter {
	public:
		TransportRouter(double bus_wait_time, double bus_velocity, transport_catalogue::TransportCatalogue& tc) :
			bus_wait_time_(bus_wait_time), bus_velocity_(bus_velocity) {
			Fill_Graphs(tc);
			route_ = std::make_unique<graph::Router<double>>(*all_graphs_.get());
		}

		std::optional<graph::Router<double>::RouteInfo> BuildRoute(graph::VertexId from, graph::VertexId to);
		graph::Edge<double> GetEdge(int edgeid) {
			return all_graphs_->GetEdge(edgeid);
		}

	private:
		void Fill_Graphs(transport_catalogue::TransportCatalogue& tc);
		std::unique_ptr<graph::DirectedWeightedGraph<double>> all_graphs_ = nullptr;
		std::unique_ptr<graph::Router<double>> route_;
		double bus_wait_time_;
		double bus_velocity_;



	};

}
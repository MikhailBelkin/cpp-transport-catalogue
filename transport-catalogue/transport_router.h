#pragma once
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"
#include <memory>




namespace transport_router {


	class TransportRouter {
	public:
		TransportRouter(double bus_wait_time, double bus_velocity, std::vector<graph::Edge<double>>& all_edges, transport_catalogue::TransportCatalogue& tc) :
			bus_wait_time_(bus_wait_time), bus_velocity_(bus_velocity) {
			
			all_graphs_ = std::make_unique<graph::DirectedWeightedGraph<double>>
				(graph::DirectedWeightedGraph<double>(tc.GetStopsCount()));

			for (graph::Edge<double> item : all_edges) {
				all_edges_.push_back(item);
				all_graphs_.get()->AddEdge(item);
			}

			
			route_ = std::make_unique<graph::Router<double>>(*all_graphs_.get());
		}


		TransportRouter(double bus_wait_time, double bus_velocity, transport_catalogue::TransportCatalogue& tc) :
			bus_wait_time_(bus_wait_time), bus_velocity_(bus_velocity) {
			Fill_Graphs(tc);
			route_ = std::make_unique<graph::Router<double>>(*all_graphs_.get());
		}

		std::optional<graph::Router<double>::RouteInfo> BuildRoute(graph::VertexId from, graph::VertexId to);
		graph::Edge<double> GetEdge(int edgeid) {
			return all_graphs_->GetEdge(edgeid);
		}

		const std::vector<graph::Edge<double>>& GetAllGraph() const {
			return all_edges_;

		}



	private:
		void Fill_Graphs(transport_catalogue::TransportCatalogue& tc);
		std::unique_ptr<graph::DirectedWeightedGraph<double>> all_graphs_ = nullptr;
		std::unique_ptr<graph::Router<double>> route_;
		std::vector<graph::Edge<double>> all_edges_;
		double bus_wait_time_;
		double bus_velocity_;



	};

}
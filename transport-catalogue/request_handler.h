#pragma once
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "geo.h"
#include "router.h"
#include "transport_catalogue.pb.h"




namespace request_queue {

	class RequestQueue {
	public:

		enum QueryType {
			ADD_STOP,
			ADD_REGULAR_BUS,
			ADD_RING_BUS,
			MAP_SETTINGS,
			ROUTING_SETTINGS,
			SERIALIZATION_SETTINGS_OUTPUT,
			SERIALIZATION_SETTINGS_INPUT,
			GET_INFO,
			BUS_INFO,
			STOP_INFO,
			MAP_INFO, 
			ROUTE_INFO
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

		enum ActivityType {
			WAITING,
			LEAVING,
			BUS_MOVING
		};



		struct RouteInfo {

			ActivityType activity;
			std::string_view name;
			int span_count=0; // количество перегонов между остановками
			double time=0;
			double total_time = 0;
		};

		struct Query {
			int id;
			QueryType query_type_;
			std::string name_;
			geo::Coordinates coordinates_;
			std::vector<std::string> stops;
			std::unordered_map<std::pair<std::string, std::string>, int, QDistHasher> distances;
			map_render::MapRenderSettings map_set;
			graph::route_settings route_set;
			
		};

		

		struct QueryResult {
			int id;
			QueryType query_type_;
			std::string name_;
			geo::Coordinates coordinates_;
			std::vector<std::string> stops;
			std::vector<std::string> buses;
			double track_lenght = 0;
			double track_distance = 0;
			
			bool found;
			
			map_render::MapRender map_data;
			std::vector<RouteInfo> route_info;


		};

		

		explicit RequestQueue(transport_catalogue::TransportCatalogue& tc) {
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
		map_render::MapRender::AllRoutes GetAllRoutes();

	private:

		std::vector<RequestQueue::RouteInfo>  BuildRoute(const RequestQueue::Query& q);
//		void Fill_Graphs();
//		std::shared_ptr<graph::DirectedWeightedGraph<double>> all_graphs_ = nullptr;
//		std::shared_ptr<graph::Router<double>> route;
		std::unique_ptr<transport_router::TransportRouter> tr_=nullptr;
		map_render::MapRender map_data_;
		graph::route_settings route_set_;
		std::unordered_map<QueryType, std::deque<Query>> requests_;
		transport_catalogue::TransportCatalogue* tc_;
	};



}



// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
/*
class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};
*/
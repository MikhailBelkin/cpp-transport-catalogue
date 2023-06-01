#include "transport_catalogue.pb.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include <string>


void Deserialize_TC(tc_serialization::TransportCatalogue& tcs, transport_catalogue::TransportCatalogue& tc);
map_render::MapRenderSettings Deserialize_Map(tc_serialization::TransportCatalogue& tcs);
graph::route_settings  Deserialize_Router(tc_serialization::TransportCatalogue& tcs, std::vector<graph::Edge<double>>& edges);
	
void Serialize_Router(std::string db_file_name, const graph::route_settings route_set, const transport_router::TransportRouter& tr);
void Serialize_Map(std::string db_file_name, const map_render::MapRenderSettings& map_set);
void Serialize_TC(std::string db_file_name, const transport_catalogue::TransportCatalogue& tc);

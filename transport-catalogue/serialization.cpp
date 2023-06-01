#include "serialization.h"
#include "geo.h"
#include <fstream>

void Deserialize_TC(tc_serialization::TransportCatalogue& tcs, transport_catalogue::TransportCatalogue &tc) {
	for (auto stop : tcs.stops()) {
		geo::Coordinates coord;
		coord.lat = stop.mutable_place()->lat();
		coord.lng = stop.mutable_place()->lng();
		transport_catalogue::Stop new_stop(stop.name(), coord);
		new_stop.SetId(stop.id());
		tc.AddStop(new_stop);
	}
	for (auto bus : tcs.buses()) {
		std::vector<const transport_catalogue::Stop*> new_bus_stops(bus.mutable_track()->size());
		for (int i = 0; i < bus.mutable_track()->size(); i++) {
			new_bus_stops[i] = tc.FindStop(bus.mutable_track(i)->name());
		}
		transport_catalogue::Bus new_bus(bus.name(), new_bus_stops);
		new_bus.SetId(bus.id());

		if (bus.is_round()) {
			new_bus.SetRound();
		}

		tc.AddBus(new_bus);

	}
	for (auto item : tcs.dist_list()) {
		auto stop1 = item.stop1();
		auto stop2 = item.stop2();
		tc.SetDistances(tc.FindStop(stop1), tc.FindStop(stop2), item.dist());
	}



}



map_render::MapRenderSettings Deserialize_Map(tc_serialization::TransportCatalogue& tcs) {

	map_render::MapRenderSettings map_set;
	/// десериализация настроек рендеринга
	map_set.width = tcs.mutable_map_settings()->width();
	map_set.height = tcs.mutable_map_settings()->height();
	map_set.padding = tcs.mutable_map_settings()->padding();
	map_set.line_with = tcs.mutable_map_settings()->line_with();
	map_set.stop_radius = tcs.mutable_map_settings()->stop_radius();
	map_set.bus_label_font_size = tcs.mutable_map_settings()->bus_label_font_size();
	map_set.bus_label_offset.first = tcs.mutable_map_settings()->bus_label_offset_first();
	map_set.bus_label_offset.second = tcs.mutable_map_settings()->bus_label_offset_second();
	map_set.stop_label_font_size = tcs.mutable_map_settings()->stop_label_font_size();
	map_set.stop_label_offset.first = tcs.mutable_map_settings()->stop_label_offset_first();
	map_set.stop_label_offset.second = tcs.mutable_map_settings()->stop_label_offset_second();

	tc_serialization::ColorVariant c;

	map_set.underlayer_color_diff.color_type = static_cast<map_render::TypeColor>(tcs.mutable_map_settings()->mutable_underlayer_color()->type());


	if (map_set.underlayer_color_diff.color_type == map_render::TypeColor::STRING) {
		map_set.underlayer_color = tcs.mutable_map_settings()->mutable_underlayer_color()->color();

	}
	if (map_set.underlayer_color_diff.color_type == map_render::TypeColor::RGB) {
		map_set.underlayer_color = svg::Rgb{
			tcs.mutable_map_settings()->mutable_underlayer_color()->red(),
			tcs.mutable_map_settings()->mutable_underlayer_color()->green(),
			tcs.mutable_map_settings()->mutable_underlayer_color()->blue()
		};
	}
	if (map_set.underlayer_color_diff.color_type == map_render::TypeColor::RGBA) {

		map_set.underlayer_color = svg::Rgba{
			tcs.mutable_map_settings()->mutable_underlayer_color()->red(),
			tcs.mutable_map_settings()->mutable_underlayer_color()->green(),
			tcs.mutable_map_settings()->mutable_underlayer_color()->blue(),
			tcs.mutable_map_settings()->mutable_underlayer_color()->opacity()
		};

	}

	map_set.underlayer_width = tcs.mutable_map_settings()->underlayer_width();

	map_set.color_palette.resize(tcs.mutable_map_settings()->color_palette_size());
	for (int i = 0; i < map_set.color_palette.size(); i++) {

		auto color_type = static_cast<map_render::TypeColor>(tcs.mutable_map_settings()->color_palette(i).type());

		if (color_type == map_render::TypeColor::STRING) {
			map_set.color_palette[i] = tcs.mutable_map_settings()->color_palette(i).color();

		}
		if (color_type == map_render::TypeColor::RGB) {
			map_set.color_palette[i] = svg::Rgb{
				tcs.mutable_map_settings()->color_palette(i).red(),
				tcs.mutable_map_settings()->color_palette(i).green(),
				tcs.mutable_map_settings()->color_palette(i).blue()
			};
		}
		if (color_type == map_render::TypeColor::RGBA) {
			map_set.color_palette[i] = svg::Rgba{
				tcs.mutable_map_settings()->color_palette(i).red(),
				tcs.mutable_map_settings()->color_palette(i).green(),
				tcs.mutable_map_settings()->color_palette(i).blue(),
				tcs.mutable_map_settings()->color_palette(i).opacity()
			};

		}

	}

	return map_set;

}


graph::route_settings  Deserialize_Router(tc_serialization::TransportCatalogue& tcs, std::vector<graph::Edge<double>>&  edges) {

	graph::route_settings route_set;

	route_set.bus_velocity = tcs.mutable_route_info()->bus_velocity();
	route_set.bus_wait_time = tcs.mutable_route_info()->bus_wait_time();
	route_set.bus_leaving_time = tcs.mutable_route_info()->bus_leaving_time();


	
	for (int i = 0; i < tcs.mutable_all_graphs()->size(); i++) {
		graph::Edge<double> item;
		item.from = tcs.mutable_all_graphs(i)->from();
		item.to = tcs.mutable_all_graphs(i)->to();
		item.weight = tcs.mutable_all_graphs(i)->weight();
		item.transport_id = tcs.mutable_all_graphs(i)->transport_id();
		item.span_count = tcs.mutable_all_graphs(i)->span_count();

		edges.push_back(item);
	}

	return route_set;
}



void Serialize_Router(std::string db_file_name, const graph::route_settings route_set, const transport_router::TransportRouter& tr) {
	tc_serialization::TransportCatalogue tcs;
	tc_serialization::RouteInfo ri;
	ri.set_bus_leaving_time(route_set.bus_leaving_time);
	ri.set_bus_velocity(route_set.bus_velocity);
	ri.set_bus_wait_time(route_set.bus_wait_time);

	*tcs.mutable_route_info() = ri;


	auto edges = tr.GetAllGraph();
	int edge_count = 0;
	for (auto item : edges) {
		tc_serialization::Edge_Route e;
		e.set_from(item.from);
		e.set_to(item.to);
		e.set_weight(item.weight);
		e.set_transport_id(item.transport_id);
		e.set_span_count(item.span_count);
		tcs.add_all_graphs();
		*tcs.mutable_all_graphs(edge_count++) = e;
	}

	std::ofstream out(db_file_name, std::ios::binary | std::ios::app);

	tcs.SerializeToOstream(&out);



}


void Serialize_Map(std::string db_file_name, const map_render::MapRenderSettings& map_set){
	tc_serialization::TransportCatalogue tcs;
	tc_serialization::MapRenderSettings mrs;
	mrs.set_width(map_set.width);
	mrs.set_height(map_set.height);
	mrs.set_padding(map_set.padding);
	mrs.set_line_with(map_set.line_with);
	mrs.set_stop_radius(map_set.stop_radius);
	mrs.set_bus_label_font_size(map_set.bus_label_font_size);
	mrs.set_bus_label_offset_first(map_set.bus_label_offset.first);
	mrs.set_bus_label_offset_second(map_set.bus_label_offset.second);
	mrs.set_stop_label_font_size(map_set.stop_label_font_size);
	mrs.set_stop_label_offset_first(map_set.stop_label_offset.first);
	mrs.set_stop_label_offset_second(map_set.stop_label_offset.second);
	tc_serialization::ColorVariant c;

	c.set_type(map_set.underlayer_color_diff.color_type);
	if (map_set.underlayer_color_diff.color_type == map_render::TypeColor::STRING) {
		c.set_color(map_set.underlayer_color_diff.color_string);
	}
	if (map_set.underlayer_color_diff.color_type == map_render::TypeColor::RGB) {
		c.set_red(map_set.underlayer_color_diff.color_r);
		c.set_green(map_set.underlayer_color_diff.color_g);
		c.set_blue(map_set.underlayer_color_diff.color_b);
	}
	if (map_set.underlayer_color_diff.color_type == map_render::TypeColor::RGBA) {
		c.set_red(map_set.underlayer_color_diff.color_r);
		c.set_green(map_set.underlayer_color_diff.color_g);
		c.set_blue(map_set.underlayer_color_diff.color_b);
		c.set_opacity(map_set.underlayer_color_diff.color_a);

	}

	*mrs.mutable_underlayer_color() = c;

	mrs.set_underlayer_width(map_set.underlayer_width);

	for (int i = 0; i < map_set.color_palette_diff.size(); i++) {
		tc_serialization::ColorVariant c;

		c.set_type(map_set.color_palette_diff[i].color_type);
		if (map_set.color_palette_diff[i].color_type == map_render::TypeColor::STRING) {
			c.set_color(map_set.color_palette_diff[i].color_string);
		}
		if (map_set.color_palette_diff[i].color_type == map_render::TypeColor::RGB) {
			c.set_red(map_set.color_palette_diff[i].color_r);
			c.set_green(map_set.color_palette_diff[i].color_g);
			c.set_blue(map_set.color_palette_diff[i].color_b);
		}
		if (map_set.color_palette_diff[i].color_type == map_render::TypeColor::RGBA) {
			c.set_red(map_set.color_palette_diff[i].color_r);
			c.set_green(map_set.color_palette_diff[i].color_g);
			c.set_blue(map_set.color_palette_diff[i].color_b);
			c.set_opacity(map_set.color_palette_diff[i].color_a);

		}

		mrs.add_color_palette();
		*mrs.mutable_color_palette(i) = c;


	}

	*tcs.mutable_map_settings() = mrs;
	std::ofstream out(db_file_name, std::ios::binary | std::ios::app);

	tcs.SerializeToOstream(&out);


}


void Serialize_TC(std::string db_file_name, const transport_catalogue::TransportCatalogue& tc) {
	tc_serialization::TransportCatalogue tcs;
	for (auto stop : tc.GetAllStops()) {
		auto new_stop = tcs.add_stops();
		new_stop->set_name(stop.GetName());
		new_stop->set_id(stop.GetId());
		new_stop->mutable_place()->set_lat(stop.GetCoordonates().lat);
		new_stop->mutable_place()->set_lng(stop.GetCoordonates().lng);
	}
	for (auto bus : tc.GetAllRoutes()) {
		auto new_bus = tcs.add_buses();
		new_bus->set_name(bus.GetName());
		new_bus->set_id(bus.GetId());
		new_bus->set_is_round(bus.isRound());
		auto track = bus.GetBusInfo();
		for (int i = 0; i < bus.GetStopsNum(); i++) {
			new_bus->add_track();
			new_bus->mutable_track(i)->set_name(track[i]->GetName()); // имени остановки достаточно
		}
	}

	auto dist = tc.GetAllDistances();
	for (auto item : dist) {
		auto d = tcs.add_dist_list();
		d->set_stop1(item.first.first);
		d->set_stop2(item.first.second);
		d->set_dist(item.second);

	}


	std::ofstream out(db_file_name, std::ios::binary);

	tcs.SerializeToOstream(&out);


}



#include <fstream>
#include <iostream>
#include <string_view>



#include "transport_catalogue.h"
#include "request_handler.h"

#include <string>
#include <iostream>
#include <cassert>

#include <chrono>
#include <sstream>
#include <string_view>
#include "graph.h"
#include "router.h"


//#include "input_reader.h"
//#include "stat_reader.h"
#include "json.h"
#include "json_reader.h"


using namespace std;
using namespace transport_catalogue;
using namespace transport;
using namespace request_queue;
using namespace transport_catalogue_input_json;
using namespace transport_catalogue_output_json;


/*
void Test_Catalogue() {

	vector<Stop> stops_base = {
			{ Stop("Stop1"s, Coordinates(10.00, 11.00)) },
			{ Stop("Stop2"s, Coordinates(12.00, 13.00)) },
			{ Stop("Stop3"s, Coordinates(10.00, 11.00)) },
			{ Stop("Stop4"s, Coordinates(10.00, 11.00)) },
			{ Stop("Stop5"s, Coordinates(10.00, 11.00)) },
			{ Stop("Stop6"s, Coordinates(59.00, 60.00)) }
	};

	StopsCatalogue stops(stops_base);
	vector <Stop*> bus1_stops = { stops.FindStop("Stop1"s), stops.FindStop("Stop3"s), stops.FindStop("Stop5"s) };
	vector <Stop*> bus2_stops = { stops.FindStop("Stop2"s), stops.FindStop("Stop4"s), stops.FindStop("Stop6"s) };
	vector<Bus> buses_base = {
		Bus("128"s, bus1_stops ),
		Bus("256"s, bus2_stops )
	};

	BusesCatalogue buses(buses_base);
	assert(stops.FindStop("Stop1"s) != nullptr);
	assert(stops.FindStop("Stop2"s) != nullptr);
	assert(stops.FindStop("Stop3"s) != nullptr);
	assert(stops.FindStop("Stop4"s) != nullptr);
	assert(stops.FindStop("Stop5"s) != nullptr);
	assert(stops.FindStop("Stop6"s) != nullptr);
	assert(stops.FindStop("Stop7"s) == nullptr);
	assert(ComputeDistance(stops.FindStop("Stop1"s)->GetCoordonates(), stops.FindStop("Stop3"s)->GetCoordonates()) == 0);
	
	assert(buses.FindBus("128"s)->GetBusInfo().size() == 3);
	assert(buses.FindBus("256"s)->GetBusInfo().size() == 3);
	assert(buses.FindBus("111"s)==nullptr);


}

void TestQueue() {
	TransportCatalogue t;
	RequestQueue rq(t);
	RequestQueue::Query q;
	
	q.query_type_ = RequestQueue::ADD_STOP;
	q.name_ = "Stop1";
	q.coordinates_ = { 10.00, 11.00 };
	rq.AddRequest(q);
	q.name_ = "Stop2";
	q.coordinates_ = { 12.00, 13.00 };
	rq.AddRequest(q);
	q.name_ = "Stop3";
	q.coordinates_ = { 14.00, 15.00 };
	rq.AddRequest(q);

	q.query_type_ = RequestQueue::ADD_REGULAR_BUS;
	q.name_ = "128";
	q.stops = { "Stop1", "Stop2", "Stop3" };
	rq.AddRequest(q);

	q.query_type_ = RequestQueue::ADD_RING_BUS;
	q.name_ = "256";
	rq.AddRequest(q);

	q.query_type_ = RequestQueue::BUS_INFO;
	q.name_ = "256";
	rq.AddRequest(q);
	
	
	vector<RequestQueue::QueryResult> res=   rq.ProcessQueue();


	//int i=0;


}
*/

/*


void TestStdInputOutput() {
	TransportCatalogue t;
	RequestQueue rq(t);
	vector<RequestQueue::Query> queries;
	cin >> queries;
	for (auto q : queries) {
		rq.AddRequest(q);
	}
	vector<RequestQueue::QueryResult> res = rq.ProcessQueue();

	cout << res;

//	int i = 0;

}

*/

void TestJson() {
	
	//auto doc = LoadJSON(inputdata);
	TransportCatalogue t;
	RequestQueue rq(t);
	std::vector<RequestQueue::Query> queries;
	//Load_query(doc, queries);
	cin >> queries;

	
	for (auto q : queries) {
		rq.AddRequest(q);
	}
	vector<RequestQueue::QueryResult> res = rq.ProcessQueue();
	

	cout << res;
	return;
}

/*void Test_Route_Building() {

	struct Weight {
		double weight = 0;
		int transoprt_id;
		bool operator==(Weight& other) {
			if (weight == other.weight && transoprt_id == other.transoprt_id) {
				return true;
			}
			return false;
		}
		bool operator>(const Weight& other) const {
			if (weight > other.weight && transoprt_id == other.transoprt_id) {
				return true;
			}
			return false;
		}
		bool operator<(const Weight& other) const {
			if (weight < other.weight && transoprt_id == other.transoprt_id) {
				return true;
			}
			return false;
		}

		Weight operator+(const Weight& other) const {
			Weight result;
			result.transoprt_id = transoprt_id;
			if (transoprt_id == other.transoprt_id) {
				result.weight += weight + other.weight;
				
			}
			else {
				//result.weight = 10000000000;
				//result.transoprt_id = other.transoprt_id;
			}
			return result;

		}

	};
	graph::DirectedWeightedGraph<Weight> all_graphs_(8);
	vector <graph::Edge<Weight>> edges = {
		{1, 2, {2.0, 1}, 1},
		{2, 3, {2.0, 1}, 1},
		{3, 4, {2.0,1}, 1},
		{4, 5, {2.0,1}, 1},
		{1, 2, {2.0, 2}, 2},
		{2, 3, {2.0, 2}, 2},
		{3, 6, {2.0,2}, 2},
		{6, 7, {2.0, 2}, 2},
		{2, 1, {2.0, 2}, 2},
		{3, 2, {2.0,2}, 2},
		{6, 3, {2.0,2} , 2},
		{7, 6, {2.0,2} , 2},
		
	};
	for (auto e : edges) {
		all_graphs_.AddEdge(e);
	}
	graph::Router<Weight> route(all_graphs_);
	auto build_result = route.BuildRoute(1, 7);
	if (build_result.has_value()) {
		graph::Router<Weight>::RouteInfo r = build_result.value();
		return;

	}
}

*/

int main1() {
	//Test_Catalogue();
	//TestQueue();
	//TestStdInputOutput();
	//TestJson();
	//Test_Route_Building();
	

	return 0;
}

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "Russian");
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
		TransportCatalogue t;
		RequestQueue rq(t);
		std::vector<RequestQueue::Query> queries;
		//Load_query(doc, queries);
		transport_catalogue_input_json::LoadQuery(json::Load(cin), queries, true);
		
		//cin >> queries;


		for (auto q : queries) {
			rq.AddRequest(q);
		}
		vector<RequestQueue::QueryResult> res = rq.ProcessQueue();

    } else if (mode == "process_requests"sv) {
		TransportCatalogue t;
		RequestQueue rq(t);
		std::vector<RequestQueue::Query> queries;
		transport_catalogue_input_json::LoadQuery(json::Load(cin), queries, false);
		//cin >> queries;


		for (auto q : queries) {
			rq.AddRequest(q);
		}
		vector<RequestQueue::QueryResult> res = rq.ProcessQueue();


		cout << res;

    } else {
        PrintUsage();
        return 1;
    }

	return 0;
}
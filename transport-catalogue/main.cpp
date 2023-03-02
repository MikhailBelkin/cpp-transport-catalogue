

#include "transport_catalogue.h"

#include <string>
#include <iostream>
#include <cassert>
#include "input_reader.h"
#include "stat_reader.h"

using namespace std;
using namespace transport_catalogue;
using namespace transport;
using namespace request_queue;
using namespace transport_catalogue_input;
using namespace transport_catalogue_output;


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

int main() {
	//Test_Catalogue();
	//TestQueue();
	TestStdInputOutput();

	return 0;
}
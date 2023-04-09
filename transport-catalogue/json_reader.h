#pragma once

#include "json.h"


#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "transport_catalogue.h"
#include <vector>
#include <deque>
#include <unordered_map>
#include <set>
#include <iomanip>
#include "request_handler.h"



namespace transport_catalogue_output_json {

	std::ostream& operator<<(std::ostream& os, std::vector<request_queue::RequestQueue::QueryResult>& query_array);
	
}


namespace transport_catalogue_input_json {


	std::istream& operator>>(std::istream& is, std::vector<request_queue::RequestQueue::Query>& query_array);

}


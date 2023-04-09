#pragma once


#include <iostream>
#include<vector>
#include <string>
#include <map>
#include "transport_catalogue.h"
#include <vector>
#include <deque>
#include <unordered_map>
#include "request_handler.h"

namespace transport_catalogue_input {

	std::istream& operator>>(std::istream& is, std::vector<request_queue::RequestQueue::Query>& query_array);

}

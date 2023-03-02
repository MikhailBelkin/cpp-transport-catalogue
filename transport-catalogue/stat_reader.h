#pragma once


#include <iostream>
#include<vector>
#include <string>
#include <map>
#include "transport_catalogue.h"
#include <set>
#include <iomanip>


namespace transport_catalogue_output {

    std::ostream& operator<<(std::ostream& os, std::vector<transport_catalogue::request_queue::RequestQueue::QueryResult>& query_array);
}
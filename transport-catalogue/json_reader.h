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



namespace transport_catalogue_output_json {

	std::ostream& operator<<(std::ostream& os, std::vector<transport_catalogue::request_queue::RequestQueue::QueryResult>& query_array);
	
}


namespace transport_catalogue_input_json {

	
	
	void Load_query(const json::Document& doc, std::vector<transport_catalogue::request_queue::RequestQueue::Query>& query_array);

	std::istream& operator>>(std::istream& is, std::vector<transport_catalogue::request_queue::RequestQueue::Query>& query_array);

}


/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
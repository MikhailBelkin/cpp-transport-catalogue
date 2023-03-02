#include "stat_reader.h"


using namespace transport_catalogue;

using namespace request_queue;
using namespace transport;


namespace transport_catalogue_output {
    std::ostream& operator<<(std::ostream& os, std::vector<transport_catalogue::request_queue::RequestQueue::QueryResult>& query_array) {
        if (query_array.empty()) {
            //os << "No buses";
            return os;
        }
        //bool new_line = false;

        for (auto element : query_array) {
            //          if (new_line) os << "\n";
            //          new_line = true;
            switch (element.query_type_) {

            case RequestQueue::BUS_INFO:
                os << "Bus " << element.name_ << ": ";
                if (element.found) {
                    std::set<std::string> uniq(element.stops.begin(), element.stops.end());
                    os << element.stops.size() << " stops on route, " << uniq.size() <<
                        " unique stops, " << std::setprecision(6) << element.track_distance << " route length, " <<
                        double(element.track_distance) / double(element.track_lenght) << " curvature";
                }
                else {
                    os << "not found";
                }

                break;

            case RequestQueue::STOP_INFO:

                os << "Stop " << element.name_ << ": ";
                if (element.found) {
                    if (element.buses.size() > 0) {
                        std::sort(element.buses.begin(), element.buses.end());
                        os << "buses";
                        for (auto bus : element.buses) {
                            os << " " << bus;
                        }
                    }
                    else {
                        os << "no buses";
                    }
                }
                else {
                    os << "not found";
                }





                break;
            default:
                break;

            }
            os << "\n";

        }
        return os;
    }

}
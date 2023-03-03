#include "stat_reader.h"


using namespace transport_catalogue;

using namespace request_queue;
using namespace transport;



void BusInfoOutput( std::ostream& os,  transport_catalogue::request_queue::RequestQueue::QueryResult& element) {
    
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

}



void StopOutputInfo(std::ostream& os, transport_catalogue::request_queue::RequestQueue::QueryResult& element) {

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


}


namespace transport_catalogue_output {
    std::ostream& operator<<(std::ostream& os, std::vector<transport_catalogue::request_queue::RequestQueue::QueryResult>& query_array) {
        if (query_array.empty()) {
            
            return os;
        }
        

        for (auto element : query_array) {
            switch (element.query_type_) {

            case RequestQueue::BUS_INFO:
                                

                BusInfoOutput(os, element);


                break;

            case RequestQueue::STOP_INFO:

                StopOutputInfo(os, element);

                    break;

            default:

                break;

            }
            
            os << "\n";

        }
        return os;
    }

}
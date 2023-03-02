#include "input_reader.h"


using namespace transport_catalogue;
using namespace request_queue;
using namespace transport;


namespace transport_catalogue_input {

	std::istream& operator>>(std::istream& is, std::vector<transport_catalogue::request_queue::RequestQueue::Query>& query_array) {
		int count_queries;
		//считываем количество запросов на добавление информации
		std::string input_str;
		if (!std::getline(is, input_str)) return is;
		count_queries = std::stoi(input_str);
		//if (!count_queries) return is;



		for (int i = 0; i < count_queries; i++) {
			std::string input_str;
			RequestQueue::Query q;



			if (!std::getline(is, input_str)) return is; // если пустой поток - завершаемся.

			std::string_view current(input_str);
			while (current[0] == ' ') current.remove_prefix(1);
			std::string_view command = current.substr(0, current.find(' '));
			if (command == "Stop") { //добавляем остановку
				current.remove_prefix(5);//удаляем слово Stop и пробел за ним
				while (current[0] == ' ') current.remove_prefix(1); // Удаляем возможные пробелы перед именем остановки??
				q.name_ = current.substr(0, current.find(':'));//имя остановки вплоть до : 

				current.remove_prefix(q.name_.size() + 1); //удаляем остановку и двоеточие

				while (current[0] == ' ') current.remove_prefix(1); // Удаляем возможные пробелы перед координатой
				std::string_view temp = current.substr(0, current.find(','));//
				q.coordinates_.lat = std::stod(std::string(temp));
				current.remove_prefix(temp.size() + 2); //удаляем первыю координату и запятую
				while (current[0] == ' ') current.remove_prefix(1); // Удаляем возможные пробелы перед второй координатой
				q.coordinates_.lng = std::stod(std::string(current.substr(0, current.find(','))));

				if (current.find_first_of(',') != std::string::npos) { // если есть еще параметры
					current.remove_prefix(current.substr(0, current.find(',')).size() + 2); //удаляем вторую координату и запятую
					while (current[0] == ' ') current.remove_prefix(1); // Удаляем возможные пробелы
				}
				else {
					current.remove_prefix(current.size()); // удаляем строку до конца, если остановка последняя
				}



				while (current.size() > 0) { // цикл по списку расстояний

					while (current[0] == ' ') current.remove_prefix(1); // Удаляем возможные пробелы перед расстоянием
					int dist = std::stoi(std::string(current.substr(0, current.find(' ') - 1))); //копируем метры без буквы m
					while (current[0] != ' ') current.remove_prefix(1); // Удаляем все до следующего пробела
					while (current[0] == ' ') current.remove_prefix(1); // Удаляем возможные пробелы
					current.remove_prefix(2); // удаляем предло to
					while (current[0] == ' ') current.remove_prefix(1); // Удаляем возможные пробелы после to
					std::string to_stop = std::string(current.substr(0, current.find(',')));// если параметр последний до запятую не найдет и и так вернет last
					q.distances[std::make_pair(q.name_, to_stop)] = dist;
					if (current.find_first_of(',') != std::string::npos) { // если есть еще параметры
						current.remove_prefix(to_stop.size() + 2); // удаляем названия остновки с запятой
						while (current[0] == ' ') current.remove_prefix(1); // Удаляем возможные пробелы
					}
					else {
						current.remove_prefix(current.size()); // удаляем строку до конца, если остановка последняя
					}
				}
				q.query_type_ = RequestQueue::ADD_STOP;
			}

			if (command == "Bus") { // добавляем австобус
				current.remove_prefix(4);//удаляем слово Bus и пробел за ним
				while (current[0] == ' ') current.remove_prefix(1); // Удаляем возможные пробелы перед номером автобуса
				q.name_ = current.substr(0, current.find(':'));//номер автобуса остановки вплоть до : 

				current.remove_prefix(q.name_.size() + 1); //удаляем номер автобуса и двоеточие


				char type_of_bus = '>'; //определяем тип разделителя и тип автобуса
				if (current.find_first_of('-') != std::string::npos) {
					type_of_bus = '-';
					q.query_type_ = RequestQueue::ADD_RING_BUS;
				}
				else {
					q.query_type_ = RequestQueue::ADD_REGULAR_BUS;
				}


				while (current.size() > 0) { // цикл по списку остановок маршрута

					while (current[0] == ' ') current.remove_prefix(1); // Удаляем возможные пробелы перед названием остановки

					std::string_view stop_name = current.substr(0, current.find(type_of_bus) - 1);//
					if (current.find_first_of(type_of_bus) != std::string::npos) {
						current.remove_prefix(current.find_first_of(type_of_bus) + 1); // удаляем текущую остановку вместе с разделителем
					}
					else {
						current.remove_prefix(current.size()); // удаляем строку до конца, если остановка последняя
					}
					while (stop_name[stop_name.size() - 1] == ' ') stop_name.remove_suffix(1); //удаляем пробелы после имени остановки
					q.stops.push_back(std::string(stop_name));


				}
			}
			query_array.push_back(q);

		}// цикл по запросам на добавление данных

		// запросы на вывод информации

		if (!std::getline(is, input_str)) return is;
		count_queries = std::stoi(input_str);
		//	if (!count_queries) return is;

		for (int i = 0; i < count_queries; i++) {
			std::string input_str;
			RequestQueue::Query q;



			if (!std::getline(is, input_str)) return is; // если пустой поток - завершаемся.

			std::string_view current(input_str);

			while (current[0] == ' ') current.remove_prefix(1);
			std::string_view command = current.substr(0, current.find(' '));

			if (command == "Bus") { //делаем запрос на вывод информации по автобусу
				current.remove_prefix(4);//удаляем слово Bus и пробел за ним
				while (current[0] == ' ') current.remove_prefix(1); // Удаляем возможные пробелы перед именем автобуса
				while (current[current.size() - 1] == ' ') current.remove_suffix(1); //удаляем пробелы после номера автобуса
				q.name_ = current;//имя автобуса
				q.query_type_ = RequestQueue::BUS_INFO;
			}
			if (command == "Stop") { //делаем запрос на вывод информации по оствановке
				current.remove_prefix(5);//удаляем слово Stop и пробел за ним
				while (current[0] == ' ') current.remove_prefix(1); // Удаляем возможные пробелы перед именем остановки
				while (current[current.size() - 1] == ' ') current.remove_suffix(1); //удаляем пробелы после имени остановки
				q.name_ = current;//имя остановки
				q.query_type_ = RequestQueue::STOP_INFO;
			}

			query_array.push_back(q);
		}
		return is;

	}

}

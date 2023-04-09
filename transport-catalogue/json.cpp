#include "json.h"

using namespace std;

namespace json {
    Number LoadNumber(std::istream& input) {
        using namespace std::literals;

        std::string parsed_num;

        // Считывает в parsed_num очередной символ из input
        auto read_char = [&parsed_num, &input] {
            parsed_num += static_cast<char>(input.get());
            if (!input) {
                throw json::ParsingError("Failed to read number from stream"s);
            }
        };

        // Считывает одну или более цифр в parsed_num из input
        auto read_digits = [&input, read_char] {
            if (!std::isdigit(input.peek())) {
                throw json::ParsingError("A digit is expected"s);
            }
            while (std::isdigit(input.peek())) {
                read_char();
            }
        };

        if (input.peek() == '-') {
            read_char();
        }
        // Парсим целую часть числа
        if (input.peek() == '0') {
            read_char();
            // После 0 в JSON не могут идти другие цифры
        }
        else {
            read_digits();
        }

        bool is_int = true;
        // Парсим дробную часть числа
        if (input.peek() == '.') {
            read_char();
            read_digits();
            is_int = false;
        }

        // Парсим экспоненциальную часть числа
        if (int ch = input.peek(); ch == 'e' || ch == 'E') {
            read_char();
            if (ch = input.peek(); ch == '+' || ch == '-') {
                read_char();
            }
            read_digits();
            is_int = false;
        }

        try {
            if (is_int) {
                // Сначала пробуем преобразовать строку в int
                try {
                    return std::stoi(parsed_num);
                }
                catch (...) {
                    // В случае неудачи, например, при переполнении,
                    // код ниже попробует преобразовать строку в double
                }
            }
            return std::stod(parsed_num);
        }
        catch (...) {
            throw json::ParsingError("Failed to convert "s + parsed_num + " to number"s);
        }
    }

    // Считывает содержимое строкового литерала JSON-документа
    // Функцию следует использовать после считывания открывающего символа ":
    std::string LoadString(std::istream& input) {
        using namespace std::literals;

        auto it = std::istreambuf_iterator<char>(input);
        auto end = std::istreambuf_iterator<char>();
        std::string s;
        while (true) {
            if (it == end) {
                // Поток закончился до того, как встретили закрывающую кавычку?
                throw json::ParsingError("String parsing error");
            }
            const char ch = *it;
            if (ch == '"') {
                // Встретили закрывающую кавычку
                ++it;
                break;
            }
            else if (ch == '\\') {
                // Встретили начало escape-последовательности
                ++it;
                if (it == end) {
                    // Поток завершился сразу после символа обратной косой черты
                    throw json::ParsingError("String parsing error");
                }
                const char escaped_char = *(it);
                // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                }
            }
            else if (ch == '\n' || ch == '\r') {
                // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                throw ParsingError("Unexpected end of line"s);
            }
            else {
                // Просто считываем очередной символ и помещаем его в результирующую строку
                s.push_back(ch);
            }
            ++it;
        }

        return s;
    }

    Node LoadNode(std::istream& input);

    Array LoadArray(std::istream& input) {
        Array result;
        char c;
        for (; input >> c && c != ']';) {
            if (c != ',') {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }
        if (c != ']') {
            throw ParsingError("Can't find end of array"s);
        }
        return result;
    }

    int LoadInt(std::istream& input) {
        int result = 0;
        while (isdigit(input.peek())) {
            result *= 10;
            result += input.get() - '0';
        }
        return result;
    }

    /*Node& LoadString(std::istream& input) {
        std::string line;
        getline(input, line, '"');
        return Node(move(line));
    }
    */
    Dict LoadDict(std::istream& input) {
        Dict result;
        char c;
        for (; input >> c && c != '}';) {
            if (c == ',') {
                input >> c;
            }

            std::string key = LoadString(input);
            input >> c;
            result.insert({ move(key), LoadNode(input) });
        }
        if (c != '}') {
            throw ParsingError("Can't find end of map"s);
        }
        return result;
    }


    bool LoadBool(std::istream& input) {
        std::string str;
        str.resize(5);
        input >> str[0];
        input >> str[1];
        input >> str[2];
        input >> str[3];
        if (str[0] == 't') {
            str.resize(4);
        }
        else {
            input >> str[4];
            str.resize(5);
        }
        
        if (str == "true") {
            return true;
        }
        else if (str == "false") {
            return false;
            }
        else{
            throw ParsingError("Unknow input"s);
        }
    }


    nullptr_t LoadNull(std::istream& input) {
        std::string str;
        str.resize(5);
        input >> str[0];
        input >> str[1];
        input >> str[2];
        input >> str[3];
        str.resize(4);

        
        if (str == "null") {
            return nullptr;
        }
        else {
            throw ParsingError("Unknow input"s);
        }
    }

    Node LoadNode(std::istream& input) {
        char c;
        input >> c;

        if (c == '[') {
            return Node(LoadArray(input));
        }
        else if (c == '{') {
            return Node(LoadDict(input));
        }
        else if (c == '"') {
            return Node(LoadString(input));
        }
        else if (c == 't' || c == 'f') {
            input.putback(c);
            return Node(LoadBool(input));
        }
        else if (c == 'n') {
            input.putback(c);
            return Node(LoadNull(input));
        }
        else{
            input.putback(c);
            auto n = LoadNumber(input);
            if (std::holds_alternative<int>(n)) return Node(std::get<int>(n));
            return Node(std::get<double>(n));
        }

    }





    namespace {
        // Шаблон, подходящий для вывода double и int

        void PrintNode(const Node& node, std::ostream& out);
       
        void PrintValue(int value, std::ostream& out) {
            out << value;
        }

        void PrintValue(double value, std::ostream& out) {
            out << value;
        }

        void PrintValue(Array value, std::ostream& out) {
            out << "\n[\n";
            bool flag = false;
            for (auto i : value) {
                if (flag) {
                    out << ",";
                }
                PrintNode(i, out);
                flag = true;
            }
            out << "\n]\n";
        }

        void PrintValue(std::string value, std::ostream& out) {
            //out << value;
            auto it = value.begin();
            auto end = value.end();
            out << "\"";
            while (it!=end) {
                //const char ch = *it;
                    //if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    //++it;
                    
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case '\n':
                        out << "\\n";
                        break;
                    case '\t':
                        out<<"\t";
                        break;
                    case '\r':
                        out<<"\\r";
                        break;
                    case '\"':
                        out<<"\\\"";
                        break;
                    case '\\':
                        out<<"\\\\";
                        break;
                    default:
                        // Просто считываем очередной символ и помещаем его в результирующую строку
                        out << *it;
                    }
                ++it;
            }

            out << "\"";

        }

        void PrintValue(bool value, std::ostream& out) {
            out << std::boolalpha<<  value;
        }

        void PrintValue(Dict value, std::ostream& out) {
            out << "\n{\n";
            bool flag = false;
            for (auto i : value) {
                if (flag) {
                    out << ", ";
                }
                out << "\""<<i.first<<"\": ";
                PrintNode(i.second, out);
                
                flag = true;

            }
            out << "\n}\n";
        }


        // Перегрузка функции PrintValue для вывода значений null
        void PrintValue(std::nullptr_t, std::ostream& out) {
            out << "null"sv;
        }

        void PrintNode(const Node& node, std::ostream& out) {
            std::visit(
                [&out](const auto& value) { PrintValue(value, out); },
                node.GetValue());
        }

        struct JsonNode {
            ostream& out;

            void operator()(nullptr_t) const {
                out << "null"sv << endl;
            }
            void operator()(double d) const {
                out << d << endl;
            }
            void operator()(int i) const {
                out << i << endl;
            }
            void operator()(Array& arr) const {
                for (const auto i : arr) {
                    PrintNode(i, out);
                }
            }
            void operator()(Dict& d) const {
                for (const auto& i : d) {
                    PrintNode(i.second, out);
                }
            }
            void operator()(std::string  s) const {
                out << s << endl;
            }
            void operator()(bool  b) const {
                out << b << endl;
            }

        };




    }  // namespace


    const Array& Node::AsArray() const {
        if (IsArray()) {
            return std::get<Array>(*this);
        }
        else {
            throw std::logic_error("Invalid value in Node");
        }

    }

    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return std::get<Dict>(*this);
        }
        else {
            throw std::logic_error("Invalid value in Node");
        }

    }

    int Node::AsInt() const {
        if (IsInt()) {
            return std::get<int>(*this);
        }
        else {
            throw std::logic_error("Invalid value in Node");
        }
    }

    const string& Node::AsString() const {
        if (IsString()) {
            return std::get<std::string>(*this);
        }
        else {
            throw std::logic_error("Invalid value in Node");
        }

    }

    bool Node::AsBool() const {
        if (IsBool()) {
            return std::get<bool>(*this);
        }
        else {
            throw std::logic_error("Invalid value in Node");
        }
    }
    double Node::AsDouble() const {
        if (IsInt()) {
            return static_cast<double>(std::get<int>(*this));
        }
        else if (IsPureDouble()) {
            return std::get<double>(*this);
        }
        else {
            throw std::logic_error("Invalid value in Node");
        }
    }

    bool Node::operator==(const Node& other) const{
        if (this->IsInt() && other.IsInt()) {
            return this->AsInt() == other.AsInt();
        }
        if (this->IsBool() && other.IsBool()) {
            return this->AsBool() == other.AsBool();
        }
        if (this->IsPureDouble() && other.IsPureDouble()) {
            return this->AsDouble() == other.AsDouble();
        }
        if (this->IsString() && other.IsString()) {
            return this->AsString() == other.AsString();
        }
        if (this->IsNull() && other.IsNull()) {
            return true;
        }
        if (this->IsArray() && other.IsArray()) {
            
            if (this->AsArray().size() != other.AsArray().size()) return false;
            if (this->AsArray().size() == 0) return true;
            auto it1 = this->AsArray().begin();
            auto it2 = other.AsArray().begin();
            for (; it1 == this->AsArray().end(); ) {
                
                Node v1 = *it1;
                Node v2 = *it2;
                if (v1 != v2) return false;
                it1++;
                it2++;
            }
            return true;
        }
        if (this->IsMap() && other.IsMap()) {

            if (this->AsMap().size() != other.AsMap().size()) return false;
            if (this->AsMap().size() == 0) return true;
            auto it1 = this->AsMap().begin();
            auto it2 = other.AsMap().begin();
            for (; it1 != this->AsMap().end(); ) {
                if (it1->first != it2->first) return false;
                Node v1 = it1->second;
                Node v2 = it2->second;
                if (v1 != v2) return false;
                it1++;
                it2++;
            }
            return true;
        }
        
        return false;

    }

bool Node::operator!=(const Node& other) const{
        return !(*this == other);
}



const NodeValue& Node::GetValue() const {
    return *this;
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const{
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), output);

    // Реализуйте функцию самостоятельно
}

bool Document::operator==(const Document& other) const {
    return root_ == other.root_;

}
bool Document::operator!=(const Document& other) const {
    return !(*this == other);
}

}  // namespace json
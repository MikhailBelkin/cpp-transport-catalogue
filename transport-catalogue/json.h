#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Number = std::variant<int, double>;
using NodeValue = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};


// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return { out, indent_step, indent_step + indent };
    }
};



class Node: private NodeValue {
public:
    using variant::variant;
    /* Реализуйте Node, используя std::variant */
    Node(NodeValue) {}
    //Node(const NodeValue v) :value_(v) {}
    const Array& AsArray() const;
    const Dict& AsMap() const;
    int AsInt() const;
    const std::string& AsString() const;
    bool AsBool() const;
    double AsDouble() const;

    const NodeValue& GetValue() const;

    

    bool IsInt() const {
        return  std::holds_alternative<int>(*this);
       
    }
    bool IsDouble() const {
        return std::holds_alternative<double>(*this)|| std::holds_alternative<int>(*this);
    }
    bool IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }
    bool IsBool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool IsString() const{
        return std::holds_alternative<std::string>(*this);
    }

    bool IsNull() const {
        return  std::holds_alternative<nullptr_t>(*this);
    }
    bool IsArray() const {
        return std::holds_alternative<Array>(*this);
    }

    bool IsMap() const {
        return std::holds_alternative<Dict>(*this);
    }

    bool operator==(const Node& other) const;
    bool operator!=(const Node& other) const;
private:
    /*Array as_array_;
    Dict as_map_;
    int as_int_ = 0;
    std::string as_string_;*/
    //NodeValue value_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;
    bool operator==(const Document& other) const;
    bool operator!=(const Document& other) const;
private:
    Node root_;
};

Document Load(std::istream& input);




void Print(const Document& doc, std::ostream& output);
std::string LoadString(std::istream& input);
Number LoadNumber(std::istream& input);


}  // namespace json
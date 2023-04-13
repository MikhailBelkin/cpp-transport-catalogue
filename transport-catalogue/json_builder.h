#pragma once

#include "json.h"
#include <stdexcept>
#include <memory>
#include <sstream>
using namespace std;
namespace json {


	class  Builder;


	class BaseContext {

	public:

		BaseContext(Builder& b) :builder_(b) {}
	protected:
		Builder& builder_;

	};


	class KeyOrEndDict :public BaseContext {
	public:
		KeyOrEndDict(Builder& b) :BaseContext(b) {}
		json::Builder& Key(Node::Value value);
		json::Builder& EndDict();

	};

	class DictContext;
	class ArrayContext;

	class ValueAfterKey :public BaseContext {
	public:
		ValueAfterKey(Builder& b) :BaseContext(b) {}
		KeyOrEndDict Value(Node::Value value);
		DictContext StartDict();
		ArrayContext StartArray();
	};


	class DictContext :public BaseContext {
	public:

		DictContext(Builder& b) :BaseContext(b) {}
		//Builder& Value(Node::Value value) = delete;


		ValueAfterKey Key(std::string key);

		json::Builder& EndDict();


	};


	class ArrayContext;


	class ValueAfterArray : public BaseContext {
	public:
		ValueAfterArray(Builder& b) :BaseContext(b) {};
		ValueAfterArray Value(Node::Value value);
		DictContext StartDict();
		ArrayContext StartArray();
		Builder& EndArray();
	};

	class ArrayContext : public BaseContext {
	public:
		ArrayContext(Builder& b) :BaseContext(b) {};

		ValueAfterArray Value(Node::Value value); //
		DictContext StartDict();
		ArrayContext StartArray();

		Builder& EndArray();
	};





	class Builder {
	public:
		Builder() {}

		Builder& Value(Node::Value value);
		Builder& Key(Node::Value value);
		DictContext StartDict();
		ArrayContext StartArray();
		Builder& EndArray();
		Builder& EndDict();


		Node Build() {
			if (nodes_stack_.empty() && !empty_) {
				return root_;
			}
			else throw std::logic_error("Can't build not ended json");
		}

		Node& GetRoot() {
			return root_;
		}



	private:
		bool empty_ = true;
		int dict_flag = 0;
		int arr_flag = 0;
		bool key_flag = false;
		Node root_;
		std::vector<std::pair<std::unique_ptr<Node>, bool>> nodes_stack_;

	};








}



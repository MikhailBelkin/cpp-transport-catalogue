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




		Builder& Value(Node::Value value) {

			Node val = value;
			//cout << "Builder_Value!"<<endl;
			//private std::variant< double, std::string> {
			if (val.IsBool() || val.IsDouble() || val.IsInt() || val.IsNull() || val.IsString() || val.IsPureDouble()) {
				if (nodes_stack_.empty()) {
					if (empty_) {
						root_ = val;
						empty_ = false;
						//nodes_stack_.push_back(make_unique<Node>(Node(value)));
						return *this;
					}
					else {
						//ноды пустые, но в руте уже записано.
						throw std::logic_error("Can't build not ended json");
					}
				}
				else {
					// значение ключа словаря, либо массива
					if ((nodes_stack_.back().first->IsString()) && key_flag) {
						//значение ключа словаря
						key_flag = false;
						nodes_stack_.push_back({ make_unique<Node>(Node(value)), true });
						return *this;
					}
					else

						if (arr_flag) {
							nodes_stack_.push_back({ make_unique<Node>(Node(value)), true });
							return *this;
						}
						else {

							throw std::logic_error("Can't build not ended json");
						}


				}
			}
			else {

				/*std::ostringstream str;

				Node temp = value;
				json::Print(
					json::Document{ temp },
					str
				);
				str << " nodes size=" << nodes_stack_.size();
				throw std::logic_error("Can't build not ended json"+str.str());*/

				if (empty_) {
					root_ = val;
					empty_ = false;
					//nodes_stack_.push_back(make_unique<Node>(Node(value)));
					return *this;
				}
				else {
					//ноды пустые, но в руте уже записано.
					throw std::logic_error("Can't build not ended json");
				}


			}
			return *this;

		}

		Builder& Key(Node::Value value) {
			//cout << "Builder_Key!" << endl;

			if (empty_) {
				throw std::logic_error("Can't make a key without dict");
			}
			if (nodes_stack_.empty()) {
				throw std::logic_error("Can't make a key without dict");

			}
			if (dict_flag <= 0) {
				throw std::logic_error("Can't make a key without dict");

			}
			if (key_flag) {
				throw std::logic_error("Can't make a key without dict");
			}
			bool right_way = false;
			//проверяем - не внутри ли мы array?
			int i = static_cast<int>(nodes_stack_.size() - 1);
			for (; i >= 0; i--) {
				if (nodes_stack_[i].first->IsArray() && !nodes_stack_[i].second) {
					throw std::logic_error("Can't make a key in Array");
				}
				if (nodes_stack_[i].first->IsDict() && !nodes_stack_[i].second) {
					right_way = true;
					break;
				}
			}

			if (!right_way) {
				throw std::logic_error("Can't make a key without Dict");
			}
			nodes_stack_.push_back({ make_unique<Node>(Node(value)), true });
			key_flag = true;


			return *this;

			//else throw std::logic_error("Can't build not ended json");
		}


		DictContext StartDict() {
			dict_flag++;
			//cout << "Builder_Start_Dict!" << endl;
			if (nodes_stack_.empty()) {
				root_ = Node(Dict());
				empty_ = false;
				nodes_stack_.push_back({ make_unique<Node>(Node(Dict())), false });
			}
			else  // если до этого был key
				if (nodes_stack_.back().first->IsString() && key_flag) {
					nodes_stack_.push_back({ make_unique<Node>(Node(Dict())), false });
					key_flag = false;
				}
				else

					if (arr_flag) {
						nodes_stack_.push_back({ make_unique<Node>(Node(Dict())), false });
					}
					else throw std::logic_error("Can't build not ended json");

			return DictContext(*this);

		}

		ArrayContext StartArray() {
			arr_flag++;
			//cout << "Builder_Start_Array!" << endl;

			if (nodes_stack_.empty()) {
				empty_ = false;
				root_ = Node(Array());
				nodes_stack_.push_back({ make_unique<Node>(Node(Array())), false });
			}
			else  // если до этого был key или это массив
				if ((nodes_stack_.back().first->IsString() && key_flag) || (arr_flag > 0)) {
					nodes_stack_.push_back({ make_unique<Node>(Node(Array())), false });
					key_flag = false;
				}
				else {
					throw std::logic_error("Can't build not ended json");
				}
			return ArrayContext(*this);

		}

		Builder& EndArray() {

			arr_flag--;
			//cout << "Builder_End_Array!" << endl;
			int i = static_cast<int>(nodes_stack_.size() - 1);
			for (; i >= 0 && !(nodes_stack_[i].first->IsArray() && !nodes_stack_[i].second); i--) {}
			if (i < 0 || !(nodes_stack_[i].first->IsArray() && !nodes_stack_[i].second)) {
				throw std::logic_error("Can't build Dict in json");
			}
			int head_arr_node = i;
			i += 1;
			Array new_arr;
			for (; i < static_cast<int>(nodes_stack_.size()); i++) {
				new_arr.emplace_back(*nodes_stack_[i].first);
			}
			i = nodes_stack_.size() - 1;
			for (; i >= head_arr_node; i--) {
				nodes_stack_.pop_back();
			}
			if (nodes_stack_.empty()) {
				root_ = new_arr;
				empty_ = false;
			}
			else {
				nodes_stack_.push_back({ make_unique<Node>(Node(Array(new_arr))), true });
			}
			return *this;


		}

		Builder& EndDict() {
			dict_flag--;
			//cout << "Builder_End_Dict!" << endl;
			int i = static_cast<int>(nodes_stack_.size() - 1);
			for (; i >= 0 && !(nodes_stack_[i].first->IsDict() && !nodes_stack_[i].second); i--) {}
			if (i < 0 || !(nodes_stack_[i].first->IsDict() && !nodes_stack_[i].second)) {
				throw std::logic_error("Can't build Dict in json");
			}
			int head_dict_node = i;
			i += 1;
			Dict new_dict;
			for (; i < static_cast<int>(nodes_stack_.size()); i += 2) {
				if ((i + 1) >= static_cast<int>(nodes_stack_.size())) {
					throw std::logic_error("Key without value");
				}
				new_dict.emplace(nodes_stack_[i].first->AsString(), *nodes_stack_[i + 1].first);
			}
			i = nodes_stack_.size() - 1;
			for (; i >= head_dict_node; i--) {
				nodes_stack_.pop_back();
			}
			if (nodes_stack_.empty()) {
				root_ = new_dict;
				empty_ = false;
			}
			else {
				nodes_stack_.push_back({ make_unique<Node>(Node(Dict(new_dict))), true });
			}
			return *this;
		}

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



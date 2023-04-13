#include "json_builder.h"

using namespace json;

ValueAfterKey   json::DictContext::Key(std::string key) {

	builder_.Key(key);
	return ValueAfterKey(builder_);
}

json::Builder& json::DictContext::EndDict() {
	return builder_.EndDict();
}

KeyOrEndDict json::ValueAfterKey::Value(Node::Value value) {
	return KeyOrEndDict(builder_.Value(value));

}


DictContext json::ValueAfterKey::StartDict() {
	return DictContext(builder_.StartDict());

}

ArrayContext json::ValueAfterKey::StartArray() {
	return ArrayContext(builder_.StartArray());
}


json::Builder& json::KeyOrEndDict::Key(Node::Value value) {
	return builder_.Key(value);
}
json::Builder& json::KeyOrEndDict::EndDict() {
	return builder_.EndDict();

}


ValueAfterArray json::ArrayContext::Value(Node::Value value) {
	return ValueAfterArray(builder_.Value(value));
}
DictContext json::ArrayContext::StartDict() {
	return DictContext(builder_.StartDict());

}
ArrayContext json::ArrayContext::StartArray() {
	return ArrayContext(builder_.StartArray());
}


Builder& json::ArrayContext::EndArray() {
	return builder_.EndArray();

}

ValueAfterArray json::ValueAfterArray::Value(Node::Value value) {
	return ValueAfterArray(builder_.Value(value));
}
DictContext json::ValueAfterArray::StartDict() {
	return DictContext(builder_.StartDict());
}
ArrayContext json::ValueAfterArray::StartArray() {
	return ArrayContext(builder_.StartArray());
}
Builder& json::ValueAfterArray::EndArray() {
	return builder_.EndArray();
}

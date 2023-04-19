#include "json_builder.h"

#include <stdexcept>
#include <iostream>

namespace json {

    Builder::DictItemContext Builder::StartDict() {
        builder_was_created_ = true;
        Dict new_dict = {};
        return CreateNewObject(new_dict);
    }
    
    Builder::KeyItemContext Builder::Key(std::string key) {
        if(!key_.empty() || has_key_){
            throw std::logic_error("Key. Incorect order for key");
        }
        if(!nodes_stack_.empty() && nodes_stack_.at(nodes_stack_.size() - 1)->IsMap()) {
            key_ = key;
            has_key_ = true;
        } else {
            throw std::logic_error("Key. Dictionary not found");
        }
        return *this;
    }
    
    Builder& Builder::EndDict() {
        if(!nodes_stack_.empty() && nodes_stack_.at(nodes_stack_.size() - 1)->IsMap() && 
           (std::get<Dict>(nodes_stack_.at(nodes_stack_.size() - 1)->GetValue()).size() == 0 || has_key_ == false)) {
            Dict new_value = std::get<Dict>(nodes_stack_.at(nodes_stack_.size() - 1)->GetValue());
            nodes_stack_.pop_back();
            if(nodes_stack_.empty()) {
                return *this;
            }
        } else {
            throw std::logic_error("EndDict. Incorrect structure");
        }
        return *this;
    }
    
    Builder::ArrayItemContext Builder::StartArray() {
        builder_was_created_ = true;
        Array new_array = {};
        return CreateNewObject(new_array);
    }
    
    Builder& Builder::EndArray() {
        
        if(!nodes_stack_.empty() && nodes_stack_.at(nodes_stack_.size() - 1)->IsArray()) {
            Array new_value = std::get<Array>(nodes_stack_.at(nodes_stack_.size() - 1)->GetValue());
            nodes_stack_.pop_back();
            if(nodes_stack_.empty()) {
                return *this;
            }
        } else {
            throw std::logic_error("EndArray. Incorrect structure");
        }
        return *this;
    }
    
    Builder& Builder::Value(Node::Value value) {
        Node new_value;
        if(std::holds_alternative<Dict>(value)) {
            new_value = std::get<Dict>(value);
        } else if(std::holds_alternative<Array>(value)) {
            new_value = std::get<Array>(value);
        } else if(std::holds_alternative<std::string>(value)) {
            new_value = std::get<std::string>(value);
        } else if(std::holds_alternative<double>(value)) {
            new_value = std::get<double>(value);
        } else if(std::holds_alternative<int>(value)) {
            new_value = std::get<int>(value);
        } else if(std::holds_alternative<bool>(value)) {
            new_value = std::get<bool>(value);
        } else if(std::holds_alternative<std::nullptr_t>(value)) {
            new_value = std::get<std::nullptr_t>(value);
        } else {
            throw std::logic_error("Value. Incorrect type for Value");
        }
        
        if(!nodes_stack_.empty() && nodes_stack_.at(nodes_stack_.size() - 1)->IsMap() && has_key_ == true) {
            Node* last = nodes_stack_.at(nodes_stack_.size() - 1);
            std::get<Dict>(last->GetValue())[key_] = new_value;
            key_.clear();
            has_key_ = false;
        } else if (!nodes_stack_.empty() && nodes_stack_.at(nodes_stack_.size() - 1)->IsArray()) {
            Node* last = nodes_stack_.at(nodes_stack_.size() - 1);
            std::get<Array>(last->GetValue()).push_back(new_value);
        } else if (nodes_stack_.empty() && !builder_was_created_) {
            builder_was_created_ = true;
            root_ = Node(new_value);
        } else {
            throw std::logic_error("Value. Incorrect type for Value");
        }
        return *this;
    }
    
    Node Builder::Build() {
        if(nodes_stack_.size() == 0 && builder_was_created_) {
            return root_;
        } else {
            throw std::logic_error("Build");
        }
    }
    
    Builder::KeyItemContext Builder::ItemContext::Key(std::string key) {
        return builder_.Key(key);
    }
    
    Builder:: DictItemContext Builder::ItemContext::StartDict() {
        return builder_.StartDict();
    }
    
    Builder& Builder::ItemContext::EndDict() {
        return builder_.EndDict();
    }
    
    Builder::ArrayItemContext Builder::ItemContext::StartArray() {
        return builder_.StartArray();
    }
    
    Builder& Builder::ItemContext::EndArray() {
        return builder_.EndArray();
    }  
    
    Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
        return builder_.Value(value);
        return ArrayItemContext(builder_);
    }
    
    Builder::ValueKeyItemContext Builder::KeyItemContext::Value(Node::Value value) {
        builder_.Value(value);
        return ValueKeyItemContext(builder_);
    }
    
}  // namespace json
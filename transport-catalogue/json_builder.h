#include "json.h"

namespace json {
    
    class Builder {
        class ItemContext;
        class KeyItemContext;
        class DictItemContext;
        class ArrayItemContext;
        class ValueKeyItemContext;
        
    public:
        template <typename Object>
        Builder& CreateNewObject(Object object);
        
        DictItemContext StartDict();
        KeyItemContext Key(std::string key);
        Builder& EndDict();
        ArrayItemContext StartArray();
        Builder& EndArray();        
        Builder& Value(Node::Value value);
        json::Node Build();
        
    private:
        Node root_;
        std::vector<Node*> nodes_stack_;   
        bool builder_was_created_ = false;
        bool has_key_;
        std::string key_;
    };
    
    template <typename Object>
    Builder& Builder::CreateNewObject(Object object) {
        if(nodes_stack_.empty()) {
            root_ = object;
            nodes_stack_.push_back(&root_);
        } else {
            Node* last = nodes_stack_.at(nodes_stack_.size() - 1);
            if(last->IsMap()) {
                if(has_key_) {
                    std::get<Dict>(last->GetValue())[key_] = Node(object);
                    nodes_stack_.push_back(&(std::get<Dict>(last->GetValue()).at(key_)));
                    key_.clear();
                    has_key_ = false;
                }
            } else if (last->IsArray()) {
                std::get<Array>(last->GetValue()).push_back(Node(object));
                int size = std::get<Array>(last->GetValue()).size();
                nodes_stack_.push_back(&(std::get<Array>(last->GetValue()).at(size -1)));
            } else {
                if(Node(object).IsMap()) {
                    throw std::logic_error("StartDict. Incorrect structure");
                } else {
                    throw std::logic_error("StartArray. Incorrect structure");
                }
            }
        }
        return *this;
    }
    
    class Builder::ItemContext {
    public:
        ItemContext(Builder& builder)
            :builder_(builder) 
            {
            }
    protected:
        DictItemContext StartDict();
        KeyItemContext Key(std::string key);
        Builder& EndDict();
        ArrayItemContext StartArray();
        Builder& EndArray();        
        
        Builder& builder_;
    };
    
    class Builder::ValueKeyItemContext : public ItemContext {
    public:
        using ItemContext::ItemContext;
        using ItemContext::Key;
        using ItemContext::EndDict;
    };
    
    class Builder::KeyItemContext : public ItemContext {
    public:
        using ItemContext::ItemContext;
        using ItemContext::StartDict;
        using ItemContext::StartArray;
        
        ValueKeyItemContext Value(Node::Value value);
        
    };
    
    class Builder::DictItemContext : public ItemContext {
    public:
        using ItemContext::ItemContext;
        using ItemContext::Key;
        using ItemContext::EndDict;
    };
    
    class Builder::ArrayItemContext : public ItemContext {
    public:
        using ItemContext::ItemContext;
        using ItemContext::StartDict;
        using ItemContext::StartArray;
        using ItemContext::EndArray;
        
        ArrayItemContext Value(Node::Value value);
    };
    
}  // namespace json
#ifndef FLUIR_UTILITY_TRIE_HPP
#define FLUIR_UTILITY_TRIE_HPP

#include <algorithm>
#include <concepts>
#include <initializer_list>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace fluir::util {
  template <std::copyable T>
  class Trie {
   public:
    using key_type = std::string_view;
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;

    Trie(const_reference defaultValue, const std::initializer_list<std::pair<key_type, value_type>>& entries);
    ~Trie();

    const_reference at(key_type key) const;
    bool contains(key_type key) const;

   private:
    struct Node;
    using nodes = std::vector<Node*>;

    struct Node {
      std::string chars;
      std::variant<nodes, value_type> value;
    };

    value_type default_{};
    Node* root_{};

    void insert(key_type key, const value_type& value, Node* node);
    value_type const* findRest(const key_type& rest, Node* node) const;
    void makeInternal(Node* node);

    void recursivelyDelete(Node* node);
  };

  template <std::copyable T>
  Trie<T>::Trie(const_reference defaultValue, const std::initializer_list<std::pair<key_type, value_type>>& entries) :
    default_(defaultValue), root_(new Node{}) {
    for (const auto& [key, value] : entries) {
      insert(key, value, root_);
    }
  }

  template <std::copyable T>
  Trie<T>::const_reference Trie<T>::at(key_type) const {
    return default_;
  }

  template <std::copyable T>
  Trie<T>::~Trie() {
    if (root_) {
      recursivelyDelete(root_);
    }
  }

  template <std::copyable T>
  bool Trie<T>::contains(key_type key) const {
    return findRest(key, root_) != nullptr;
  }

  template <std::copyable T>
  void Trie<T>::insert(key_type key, const value_type& value, Node* node) {
    if (node->chars.empty()) {
      node->chars = key;
      node->value = value;
    } else {
      if (std::holds_alternative<nodes>(node->value)) {
        auto& children = std::get<nodes>(node->value);
        auto firstChar = std::ranges::find(node->chars, key[0]);
        if (firstChar == node->chars.end()) {
          node->chars.push_back(key[0]);
          children.emplace_back(new Node{std::string{key.substr(1)}, value});
        } else {
          auto index = std::distance(node->chars.begin(), firstChar);
          insert(key.substr(1), value, children[index]);
        }
      } else {
        makeInternal(node);
        insert(key, value, node);
      }
    }
  }

  template <std::copyable T>
  Trie<T>::value_type const* Trie<T>::findRest(const key_type& rest, Node* node) const {
    if (!node) {
      return nullptr;
    }
    if (std::holds_alternative<value_type>(node->value)) {
      if (rest != node->chars) {
        return nullptr;
      }
      auto& value = std::get<value_type>(node->value);
      return &value;
    } else {
      auto& children = std::get<nodes>(node->value);
      auto firstChar = std::ranges::find(node->chars, rest[0]);
      if (firstChar == node->chars.end()) {
        return nullptr;
      }
      auto index = std::distance(node->chars.begin(), firstChar);
      return findRest(rest.substr(1), children[index]);
    }
    return nullptr;
  }

  template <std::copyable T>
  void Trie<T>::makeInternal(Node* node) {
    auto child = new Node{};
    child->chars = node->chars.substr(1);
    child->value = std::move(node->value);
    node->chars = node->chars[0];
    node->value = nodes{child};
  }

  template <std::copyable T>
  void Trie<T>::recursivelyDelete(Node* node) {
    if (std::holds_alternative<nodes>(node->value)) {
      auto& children = std::get<nodes>(node->value);
      for (auto child : children) {
        recursivelyDelete(child);
      }
    }
    delete node;
  }
}  // namespace fluir::util
#endif

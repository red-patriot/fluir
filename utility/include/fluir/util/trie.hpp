#ifndef FLUIR_UTILITY_TRIE_HPP
#define FLUIR_UTILITY_TRIE_HPP

#include <algorithm>
#include <concepts>
#include <initializer_list>
#include <memory>
#include <string>
#include <utility>

namespace fluir::util {
  template <std::copyable T>
  class Trie {
   public:
    using key_type = std::string_view;
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;

    Trie(const_reference defaultValue, const std::initializer_list<std::pair<key_type, value_type>>& entries);

    const_reference at(key_type key) const;
    bool contains(key_type key) const;

   private:
    struct Node {
      std::string chars;
      value_type value;
    };

    value_type default_{};
    std::unique_ptr<Node> root_{};

    void insert(const key_type& key, const value_type& value, Node* node);
    value_type const* findRest(const key_type& rest, Node* node) const;
  };

  template <std::copyable T>
  Trie<T>::Trie(const_reference defaultValue, const std::initializer_list<std::pair<key_type, value_type>>& entries) :
    default_(defaultValue), root_(std::make_unique<Node>()) {
    for (const auto& [key, value] : entries) {
      insert(key, value, root_.get());
    }
  }

  template <std::copyable T>
  Trie<T>::const_reference Trie<T>::at(key_type) const {
    return default_;
  }

  template <std::copyable T>
  bool Trie<T>::contains(key_type key) const {
    return findRest(key, root_.get()) != nullptr;
  }

  template <std::copyable T>
  void Trie<T>::insert(const key_type& key, const value_type& value, Node* node) {
    if (node->chars.empty()) {
      node->chars = key;
      node->value = value;
    }
  }

  template <std::copyable T>
  Trie<T>::value_type const* Trie<T>::findRest(const key_type& rest, Node* node) const {
    if (!node) {
      return nullptr;
    }
    if (node->chars == rest) {
      return &node->value;
    }
    return nullptr;
  }
}  // namespace fluir::util
#endif

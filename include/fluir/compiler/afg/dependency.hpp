#ifndef FLUIR_COMPILER_AFG_DEPENDENCY_HPP
#define FLUIR_COMPILER_AFG_DEPENDENCY_HPP

#include "fluir/compiler/afg/node.hpp"

namespace fluir::afg {
  /** Represents a (potentially shared) dependency on a Node's output */
  class Dependency {
    using UnderlyingNode = std::shared_ptr<UniqueNode>;

   public:
    Dependency(UnderlyingNode source) :
        source_(std::move(source)) { }
    Dependency(const Dependency&) = default;
    Dependency& operator=(const Dependency&) = default;
    // Dependency(Dependency&&); <- SHOULD COPY
    // Dependency& operator=(Dependency&&); <- SHOULD COPY
    ~Dependency() = default;

    Node& operator*() { return **source_; }
    const Node& operator*() const { return **source_; }
    Node* operator->() { return source_->get(); }
    Node const* const operator->() const { return source_->get(); }

    Node* get() { return source_->get(); }
    Node const* const get() const { return source_->get(); }

    /** Gets the number of upstream nodes that share this dependency */
    size_t upstreamCount() const { return source_.use_count(); }
    /** Indicates if onle a single node holds this dependency */
    bool unique() const { return upstreamCount() == 1; }

    /** Reset all co-dependers to a new source dependency */
    template <ConcreteNode NewSource, typename... Args>
    void reset(Args&&... args) {
      *source_ = std::make_unique<NewSource>(std::forward<Args>(args)...);
    }

   private:
    UnderlyingNode source_;

    template <ConcreteNode NewSource>
    friend Dependency makeDependency(NewSource n);
  };

  template <ConcreteNode Source, typename... Args>
  inline Dependency makeDependency(Args&&... args) {
    return Dependency{
        std::make_shared<UniqueNode>(
            std::make_unique<Source>(
                std::forward<Args>(args)...))};
  }

}  // namespace fluir::afg

#endif

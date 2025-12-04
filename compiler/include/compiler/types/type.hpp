#ifndef FLUIR_COMPILER_TYPES_TYPE_HPP
#define FLUIR_COMPILER_TYPES_TYPE_HPP

#include <string>

namespace fluir::types {
  /** The description of a type in the Fluir language */
  class Type {
   public:
    explicit Type(std::string name);

    /** Access the unqualified name of this type */
    const std::string& name() const { return name_; }

    friend bool operator==(const Type&, const Type&) = default;

   private:
    std::string name_; /**< The unqualified name of the type */
  };
}  // namespace fluir::types

#endif

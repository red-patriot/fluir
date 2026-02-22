#ifndef FLUIR_COMPILER_TYPES_FUNCTION_TYPE_HPP
#define FLUIR_COMPILER_TYPES_FUNCTION_TYPE_HPP

#include <optional>
#include <string>
#include <vector>

#include "compiler/types/typeid.hpp"

namespace fluir::types {

  /** Represents the type signature of a function: its parameter types and optional return type */
  class FunctionType {
   public:
    FunctionType(std::string name, std::vector<TypeID> parameters, std::optional<TypeID> returnType) :
      name_(std::move(name)), parameters_(std::move(parameters)), returnType_(returnType) { }

    [[nodiscard]] const std::string& name() const { return name_; }
    [[nodiscard]] const std::vector<TypeID>& parameters() const { return parameters_; }
    [[nodiscard]] std::optional<TypeID> returnType() const { return returnType_; }

   private:
    std::string name_;
    std::vector<TypeID> parameters_;
    std::optional<TypeID> returnType_;
  };

}  // namespace fluir::types

#endif

#ifndef FLUIR_COMPILER_AFG_BUILDER_HPP
#define FLUIR_COMPILER_AFG_BUILDER_HPP

#include "fluir/compiler/afg.hpp"
#include "fluir/compiler/parse_tree.hpp"

namespace fluir::compiler {
  fluir::afg::FlowGraph buildGraphOf(fluir::parse_tree::Block);
}

#endif

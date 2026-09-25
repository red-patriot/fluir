#ifndef FLUIR_EDITOR_CORE_TREE_HPP
#define FLUIR_EDITOR_CORE_TREE_HPP

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"

namespace fluir::editor {

  /** A branch's path segment is its 1-based index in its conditional; the model holds no id. */
  inline constexpr fluir::ID THEN_BRANCH_ID = 1;
  inline constexpr fluir::ID ELSE_BRANCH_ID = 2;

  /** The editor's annotated parse tree. Every `et::` name the editor needs is re-exported here, so editor code never
   *  names the unannotated instantiation by accident. */
  namespace et {

    /** Ephemeral editor state hung on the tree: never parsed, never written, never undone. */
    struct Annotations {
      struct Conditional {
        fluir::ID shownBranch = THEN_BRANCH_ID; /**< Which branch the frame lays out and draws. */

        friend bool operator==(const Conditional&, const Conditional&) = default;
      };
      struct Comment {
        friend bool operator==(const Comment&, const Comment&) = default;
      };
      struct Constant {
        friend bool operator==(const Constant&, const Constant&) = default;
      };
      struct Binary {
        friend bool operator==(const Binary&, const Binary&) = default;
      };
      struct Unary {
        friend bool operator==(const Unary&, const Unary&) = default;
      };
      struct Call {
        friend bool operator==(const Call&, const Call&) = default;
      };
      struct FunctionDecl {
        friend bool operator==(const FunctionDecl&, const FunctionDecl&) = default;
      };
      struct ParseTree {
        friend bool operator==(const ParseTree&, const ParseTree&) = default;
      };
    };

    // The annotated instantiation.
    using Comment = pt::CommentT<Annotations>;
    using Constant = pt::ConstantT<Annotations>;
    using Binary = pt::BinaryT<Annotations>;
    using Unary = pt::UnaryT<Annotations>;
    using Call = pt::CallT<Annotations>;
    using Conditional = pt::ConditionalT<Annotations>;
    using Node = pt::NodeT<Annotations>;
    using Block = pt::BlockT<Annotations>;
    using FunctionDecl = pt::FunctionDeclT<Annotations>;
    using Declaration = pt::DeclarationT<Annotations>;
    using ParseTree = pt::ParseTreeT<Annotations>;
    using Tree = ParseTree;

    // Straight re-exports: these carry no annotation and are shared with the plain instantiation.
    using pt::BlockPort;
    using pt::BlockPorts;
    using pt::Conduit;
    using pt::Header;
    using Literal = pt::Literal;

  }  // namespace et

}  // namespace fluir::editor

#endif

# Test Programs

This directory contains various example programs to test various scenarios in the components that read from files.
It is intended to be a shared source of truth for what constitutes a valid or invalid fluir source file.

Subdirectories:

|                  | Description                                                                        |
|------------------|------------------------------------------------------------------------------------|
| ast              | Test programs which are expected to result in a valid AST.                         |
| build_ast_errors | Test Programs which have valid syntax, but which do not produce a valid AST.       |
| parse            | Test programs which have valid syntax.                                             |
| syntax_error     | Test programs with syntax errors.                                                  |
| type_check       | Test programs which are expected to produce a valid AST with fully-resolved types. |
| type_errors      | Test programs which are expected to fail during type checking.                     |

In the above table, the valid golden files are agnostic to later stages of the semantic pipeline. E.g. The programs
in `parse` must not have syntax errors, but they may not demonstrate programs producing a valid AST or for which
type checking succeeds. Likewise, programs in `ast` will build an ast, but type checking may or may not fail. The latter
steps are not part of the demonstration.

The golden criteria also specifies that error exemplars shall fail at the specified stage. E.g. `type_error` programs
shall not fail during parsing or AST construction.

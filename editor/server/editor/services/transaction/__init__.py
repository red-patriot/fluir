from editor.services.transaction.add_conduit import AddConduit
from editor.services.transaction.add_decl import AddDecl, FunctionParams
from editor.services.transaction.add_decl_interface import (
    AddDeclInterface,
    DeclParameterParams,
    DeclReturnParams,
)
from editor.services.transaction.add_node import (
    AddNode,
    ConstantParams,
    OperatorParams,
)
from editor.services.transaction.base import TransactionBase
from editor.services.transaction.move import MoveElement
from editor.services.transaction.remove import RemoveItem
from editor.services.transaction.rename import RenameDeclaration
from editor.services.transaction.resize import ResizeElement
from editor.services.transaction.update_constant import UpdateConstant
from editor.services.transaction.update_operator import UpdateOperator

type EditTransaction = (
    MoveElement
    | ResizeElement
    | RenameDeclaration
    | UpdateConstant
    | UpdateOperator
    | AddConduit
    | AddNode
    | AddDecl
    | AddDeclInterface
    | RemoveItem
)

__all__ = [
    "TransactionBase",
    "EditTransaction",
    "MoveElement",
    "ResizeElement",
    "RenameDeclaration",
    "UpdateConstant",
    "UpdateOperator",
    "AddConduit",
    "AddNode",
    "AddDecl",
    "AddDeclInterface",
    "DeclParameterParams",
    "DeclReturnParams",
    "FunctionParams",
    "ConstantParams",
    "OperatorParams",
    "RemoveItem",
]

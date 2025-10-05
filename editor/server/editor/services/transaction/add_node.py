from typing import Literal, override

from pydantic import BaseModel

from editor.models import Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import find_element
from editor.models.id import IDType
from editor.services.transaction.base import TransactionBase
from editor.services.transaction.remove import RemoveItem
from editor.utility.next_id import next_id

_AddOption = Literal[
    "BinaryOperator",
    "UnaryOperator",
    "F64",
    "I8",
    "I16",
    "I32",
    "I64",
    "U8",
    "U16",
    "U32",
    "U64",
]


class AddNode(BaseModel, TransactionBase):
    discriminator: Literal["add_node"] = "add_node"
    parent: QualifiedID
    new_type: _AddOption
    new_location: elements.Location
    _inserted: IDType | None = None

    @override
    def do(self, original: Program) -> Program:
        decl = find_element(self.parent, original)
        if not isinstance(decl, elements.Function):
            # TODO: Handle other types of decls better
            raise BadEdit("Source must be a function to add a conduit")
        new_id = next_id(decl)

        match self.new_type:
            case "F64":
                decl.nodes.append(
                    elements.Constant(
                        id=new_id,
                        location=self.new_location,
                        value="0.0",
                        flType=elements.FlType.F64,
                    )
                )
            case "I8" | "I16" | "I32" | "I64" | "U8" | "U16" | "U32" | "U64":
                decl.nodes.append(
                    elements.Constant(
                        id=new_id,
                        location=self.new_location,
                        value="0",
                        flType=elements.FlType(self.new_type),
                    )
                )
            case "BinaryOperator":
                decl.nodes.append(
                    elements.BinaryOperator(
                        id=new_id,
                        location=self.new_location,
                        # TODO: Accept the operator as an input?
                        op=elements.Operator.PLUS,
                    )
                )
            case "UnaryOperator":
                decl.nodes.append(
                    elements.UnaryOperator(
                        id=new_id,
                        location=self.new_location,
                        # TODO: Accept the operator as an input?
                        op=elements.Operator.PLUS,
                    )
                )
        self._inserted = new_id
        return original

    @override
    def undo(self, original: Program) -> Program:
        assert self._inserted is not None
        return RemoveItem(target=self.parent + [self._inserted]).do(original)

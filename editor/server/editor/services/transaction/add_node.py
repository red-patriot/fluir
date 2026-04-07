from typing import Literal, override

from pydantic import BaseModel

from editor.models import Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import find_element
from editor.models.id import IDType
from editor.services.transaction.base import TransactionBase
from editor.services.transaction.remove import RemoveItem
from editor.utility.next_id import next_id

_AddOption = Literal["call", "constant", "operator"]


class AddNode(BaseModel, TransactionBase):
    discriminator: Literal["add_node"] = "add_node"
    parent: QualifiedID
    new_type: _AddOption
    new_location: elements.Location
    # Optional additional parameters to use when constructing the node
    data: dict[str, str] = {}
    _inserted: IDType | None = None

    @override
    def do(self, original: Program) -> Program:
        decl = find_element(self.parent, original)
        if not isinstance(decl, elements.Function):
            # TODO: Handle other types of decls better
            raise BadEdit("Source must be a function to add a conduit")
        new_id = next_id(decl)

        match self.new_type:
            case "constant":
                decl.nodes.append(self._make_constant(new_id))
            case "operator":
                decl.nodes.append(self._make_operator(new_id))
        self._inserted = new_id
        return original

    def _make_constant(self, new_id: IDType) -> elements.Constant:
        if "type" not in self.data:
            raise BadEdit("'constant' node data requires a 'type' element")
        if not isinstance(self.data["type"], str):
            raise BadEdit("'constant' node data['type'] must be a string")
        fl_type = elements.FlType(self.data["type"])
        value = "0.0" if fl_type == elements.FlType.F64 else "0"
        return elements.Constant(
            id=new_id,
            location=self.new_location,
            value=value,
            flType=fl_type,
        )

    def _make_operator(
        self, new_id: IDType
    ) -> elements.BinaryOperator | elements.UnaryOperator:
        if "arity" not in self.data:
            raise BadEdit("'operator' node data requires a 'arity' element")
        match self.data["arity"]:
            case "binary":
                return elements.BinaryOperator(
                    id=new_id,
                    location=self.new_location,
                    op=elements.Operator.PLUS,
                )
            case "unary":
                return elements.UnaryOperator(
                    id=new_id,
                    location=self.new_location,
                    op=elements.Operator.PLUS,
                )
            case arity:
                raise BadEdit(f"Unknown arity: {arity}")

    @override
    def undo(self, original: Program) -> Program:
        assert self._inserted is not None
        return RemoveItem(target=self.parent + [self._inserted]).do(original)

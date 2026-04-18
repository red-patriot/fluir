from typing import Annotated, Literal, override

from pydantic import BaseModel, Field

from editor.models import Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import find_element
from editor.models.id import IDType
from editor.services.transaction.base import TransactionBase
from editor.services.transaction.remove import RemoveItem
from editor.utility.next_id import next_id


class ConstantParams(BaseModel):
    discriminator: Literal["constant"] = "constant"
    type: elements.FlType = elements.FlType.F64
    value: str | None = None


class OperatorParams(BaseModel):
    discriminator: Literal["operator"] = "operator"
    arity: Literal["binary", "unary"]
    op: str | None = None


class CallParams(BaseModel):
    discriminator: Literal["call"] = "call"
    target: str | None = None


type NodeParams = Annotated[
    ConstantParams | OperatorParams | CallParams,
    Field(discriminator="discriminator"),
]


class AddNode(BaseModel, TransactionBase):
    discriminator: Literal["add_node"] = "add_node"
    parent: QualifiedID
    new_location: elements.Location
    params: NodeParams
    _inserted: IDType | None = None

    @override
    def do(self, original: Program) -> Program:
        decl = find_element(self.parent, original)
        if not isinstance(decl, elements.Function):
            # TODO: Handle other types of decls better
            raise BadEdit("Source must be a function to add a conduit")
        new_id = next_id(decl)

        match self.params:
            case ConstantParams():
                decl.nodes.append(self._make_constant(new_id))
            case OperatorParams():
                decl.nodes.append(self._make_operator(new_id))
            case CallParams():
                decl.nodes.append(self._make_call(new_id, original))
        self._inserted = new_id
        return original

    def _make_constant(self, new_id: IDType) -> elements.Constant:
        assert isinstance(self.params, ConstantParams)
        fl_type = self.params.type
        value = self.params.value
        if value is None:
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
        assert isinstance(self.params, OperatorParams)
        op = self._resolve_operator()
        match self.params.arity:
            case "binary":
                return elements.BinaryOperator(
                    id=new_id,
                    location=self.new_location,
                    op=op,
                )
            case "unary":
                return elements.UnaryOperator(
                    id=new_id,
                    location=self.new_location,
                    op=op,
                )

    def _resolve_operator(self) -> elements.Operator:
        assert isinstance(self.params, OperatorParams)
        if self.params.op is None:
            return elements.Operator.PLUS
        try:
            op = elements.Operator(self.params.op)
        except ValueError:
            raise BadEdit(f"Unknown operator: '{self.params.op}'")
        if op == elements.Operator.UNKNOWN:
            return elements.Operator.PLUS
        return op

    def _make_call(self, new_id: IDType, original: Program) -> elements.Call:
        assert isinstance(self.params, CallParams)
        target_name = self.params.target if self.params.target else "???"
        targets = [
            decl for decl in original.declarations if decl.name == target_name
        ]
        args = []
        returns = False
        if len(targets) == 1:
            target = targets[0]
            args = [param.name for param in target.inputs]
            # TODO: Handle multiple returns
            returns = len(target.outputs) == 1

        return elements.Call(
            id=new_id,
            location=self.new_location,
            target=target_name,
            arguments=args,
            returns=returns,
        )

    @override
    def undo(self, original: Program) -> Program:
        assert self._inserted is not None
        return RemoveItem(target=self.parent + [self._inserted]).do(original)

from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import Literal, cast, override

from pydantic import BaseModel

from editor.models import Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import find_element, find_item
from editor.models.id import make_qualified_id
from editor.utility.next_id import next_id


class TransactionBase(ABC):
    """A transaction that can be accepted from the client to edit a program"""

    @abstractmethod
    def do(self, original: Program) -> Program:
        """Apply an operation to the given program and return the resulting program"""

    @abstractmethod
    def undo(self, original: Program) -> Program:
        """Undoes the operation performed in `do`"""


@dataclass
class _Limits:
    upper: int
    lower: int


class MoveElement(BaseModel, TransactionBase):
    discriminator: Literal["move"] = "move"
    target: QualifiedID
    x: int
    y: int

    @override
    def do(self, original: Program) -> Program:
        element = find_element(self.target, original)
        new_x = self.x
        new_y = self.y
        if len(self.target) > 1:
            xlim, ylim = self._get_limits(original)
            if (
                new_x < xlim.lower
                or new_y < ylim.lower
                or (new_x + element.location.width) > xlim.upper
                or (new_y + element.location.height) > ylim.upper
            ):
                raise BadEdit("Element cannot be moved out of bounds")
        element.location.x = new_x
        element.location.y = new_y

        return original

    def _get_limits(self, program: Program) -> tuple[_Limits, _Limits]:
        enclosing = find_element(self.target[:-1], program)
        return (
            _Limits(upper=enclosing.location.width, lower=0),
            _Limits(upper=enclosing.location.height, lower=0),
        )

    @override
    def undo(self, original: Program) -> Program:
        return original


class ResizeElement(BaseModel, TransactionBase):
    discriminator: Literal["resize"] = "resize"
    target: QualifiedID
    width: int
    height: int

    @override
    def do(self, original: Program) -> Program:
        element = find_element(self.target, original)
        limits = self._get_limits(original)
        if limits:
            horizontal, vertical = limits
            self.width = max(
                horizontal.lower,
                min(self.width, horizontal.upper - element.location.x),
            )
            self.height = max(
                vertical.lower,
                min(self.height, vertical.upper - element.location.y),
            )
        element.location.width = self.width
        element.location.height = self.height
        return original

    @override
    def undo(self, original: Program) -> Program:
        return original

    def _get_limits(self, program: Program) -> tuple[_Limits, _Limits] | None:
        if len(self.target) <= 1:
            # Elements with no parent have no limits
            return None
        enclosing = find_element(self.target[:-1], program)
        return (
            _Limits(upper=enclosing.location.width, lower=4),
            _Limits(upper=enclosing.location.height, lower=4),
        )


class RenameDeclaration(BaseModel, TransactionBase):
    discriminator: Literal["rename_declaration"] = "rename_declaration"
    target: QualifiedID
    name: str

    @override
    def do(self, original: Program) -> Program:
        decl = find_element(self.target, original)
        if not isinstance(decl, elements.Declaration):
            raise BadEdit("Only a declaration may be renamed")
        decl.name = self.name
        return original

    @override
    def undo(self, original: Program) -> Program:
        return original


class UpdateConstant(BaseModel, TransactionBase):
    discriminator: Literal["update_constant"] = "update_constant"
    target: QualifiedID
    value: str

    @override
    def do(self, original: Program) -> Program:
        element = find_element(self.target, original)
        if not isinstance(element, elements.Constant):
            raise BadEdit("Can only perform this update on a constant")

        element.value = self.value
        # TODO: Update type here too?

        return original

    @override
    def undo(self, original: Program) -> Program:
        return original


class UpdateOperator(BaseModel, TransactionBase):
    discriminator: Literal["update_operator"] = "update_operator"
    target: QualifiedID
    value: str

    @override
    def do(self, original: Program) -> Program:
        element = find_element(self.target, original)
        if not (
            isinstance(element, elements.BinaryOperator)
            or isinstance(element, elements.UnaryOperator)
        ):
            raise BadEdit("Can only perform this update on an operator")

        element.op = elements.Operator(self.value)
        return original

    @override
    def undo(self, original: Program) -> Program:
        return original


class AddConduit(BaseModel, TransactionBase):
    discriminator: Literal["add_conduit"] = "add_conduit"
    source: str  # "input-QualifiedID-index"
    target: str  # "output-QualifiedID-index"

    @dataclass
    class InOutID:
        id: QualifiedID
        index: int

    @override
    def do(self, original: Program) -> Program:
        # Parse the source and target strings to get the IDs and indices
        source_parts = self.source.split("-")
        target_parts = self.target.split("-")
        if len(source_parts) != 3 or len(target_parts) != 3:
            raise BadEdit("Invalid source or target format for conduit")
        source_id = self.InOutID(
            id=make_qualified_id(source_parts[1]),
            index=int(source_parts[2]),
        )
        target_id = self.InOutID(
            id=make_qualified_id(target_parts[1]),
            index=int(target_parts[2]),
        )

        # Create a conduit and add it to the program
        if source_id.id[:-1] != target_id.id[:-1]:
            # TODO: Handle this better
            raise BadEdit("Source and target must be in the same function")

        decl = find_element(source_id.id[:-1], original)
        if not isinstance(decl, elements.Function):
            # TODO: Handle other types of decls better
            raise BadEdit("Source must be a function to add a conduit")

        if existing_conduit := next(
            (
                conduit
                for conduit in decl.conduits
                if len(conduit.children) > 0
                and isinstance(conduit.children[0], elements.Conduit.Output)
                and conduit.children[0].target == target_id.id[-1]
                and conduit.children[0].index == target_id.index
            ),
            None,
        ):
            # If a conduit already exists, we reset the source
            existing_conduit.input = source_id.id[-1]
            existing_conduit.index = source_id.index
            return original

        decl.conduits.append(
            elements.Conduit(
                id=next_id(decl),  # ID should be generated by the system
                input=source_id.id[-1],
                index=source_id.index,
                children=[
                    elements.Conduit.Output(
                        target=target_id.id[-1], index=target_id.index
                    )
                ],
            )
        )

        return original

    @override
    def undo(self, original: Program) -> Program:
        return original


class AddNode(BaseModel, TransactionBase):
    discriminator: Literal["add_node"] = "add_node"
    parent: QualifiedID
    new_type: Literal["Constant", "BinaryOperator", "UnaryOperator"]
    new_location: elements.Location

    @override
    def do(self, original: Program) -> Program:
        decl = find_element(self.parent, original)
        if not isinstance(decl, elements.Function):
            # TODO: Handle other types of decls better
            raise BadEdit("Source must be a function to add a conduit")
        new_id = next_id(decl)

        match self.new_type:
            case "Constant":
                decl.nodes.append(
                    elements.Constant(
                        id=new_id,
                        location=self.new_location,
                        value="0.0",
                        flType=elements.FlType.FLOATING_POINT,
                    )
                )
            case "BinaryOperator":
                decl.nodes.append(
                    elements.BinaryOperator(
                        id=new_id,
                        location=self.new_location,
                        op=elements.Operator.UNKNOWN,
                    )
                )
            case "UnaryOperator":
                decl.nodes.append(
                    elements.UnaryOperator(
                        id=new_id,
                        location=self.new_location,
                        op=elements.Operator.UNKNOWN,
                    )
                )

        return original

    @override
    def undo(self, original: Program) -> Program:
        return original


class RemoveItem(BaseModel, TransactionBase):
    target: QualifiedID

    @override
    def do(self, original: Program) -> Program:
        if len(self.target) < 1:
            raise BadEdit("Cannot remove the program itself")

        if len(self.target) == 1:
            # Removing a function from the program
            original.declarations = [
                func
                for func in original.declarations
                if func.id != self.target[0]
            ]
            return original

        parent_id = self.target[:-1]
        parent = find_item(parent_id, original)
        element_id = self.target[-1]

        if isinstance(parent, elements.Function):
            # Remove from nodes list
            parent.nodes = [
                node for node in parent.nodes if node.id != element_id
            ]
            parent.conduits = [
                conduit
                for conduit in parent.conduits
                if conduit.id != element_id
            ]
            # Remove any conduits connected to this node
            parent.conduits = [
                conduit
                for conduit in parent.conduits
                if conduit.input != element_id
                and not any(
                    cast(elements.Conduit.Output, out).target == element_id
                    for out in conduit.children
                )
            ]
        else:
            raise BadEdit("Can only remove elements from functions")

        return original

    @override
    def undo(self, original: Program) -> Program:
        return original


type EditTransaction = (
    MoveElement
    | ResizeElement
    | RenameDeclaration
    | UpdateConstant
    | UpdateOperator
    | AddConduit
    | AddNode
    | RemoveItem
)

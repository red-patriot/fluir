import copy
from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import Literal, cast, override

from pydantic import BaseModel

from editor.models import Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import find_element, find_item
from editor.models.id import IDType, make_qualified_id
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
        self.x = element.location.x
        self.y = element.location.y

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
        element = find_element(self.target, original)
        element.location.x, self.x = self.x, element.location.x
        element.location.y, self.y = self.y, element.location.y
        return original


class ResizeElement(BaseModel, TransactionBase):
    discriminator: Literal["resize"] = "resize"
    target: QualifiedID
    width: int
    height: int

    @override
    def do(self, original: Program) -> Program:
        element = find_element(self.target, original)
        limits = self._get_limits(original, element)
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
        new_width, self.width = self.width, element.location.width
        new_height, self.height = self.height, element.location.height

        element.location.width = new_width
        element.location.height = new_height
        return original

    @override
    def undo(self, original: Program) -> Program:
        element = find_element(self.target, original)
        new_width, self.width = self.width, element.location.width
        new_height, self.height = self.height, element.location.height

        element.location.width = new_width
        element.location.height = new_height
        return original

    def _get_limits(
        self, program: Program, element: elements.Element
    ) -> tuple[_Limits, _Limits] | None:
        ret = (
            _Limits(upper=1000, lower=4),
            _Limits(upper=1000, lower=4),
        )

        if isinstance(element, elements.Declaration):
            # TODO: Handle other declarations
            ret[0].lower = max(
                (
                    node.location.x + node.location.width
                    for node in element.nodes
                ),
                default=15,
            )
            ret[1].lower = max(
                (
                    node.location.y + node.location.height
                    for node in element.nodes
                ),
                default=15,
            )
        if len(self.target) > 1:
            enclosing = find_element(self.target[:-1], program)
            ret[0].upper = enclosing.location.width
            ret[1].upper = enclosing.location.height
        return ret


class RenameDeclaration(BaseModel, TransactionBase):
    discriminator: Literal["rename_declaration"] = "rename_declaration"
    target: QualifiedID
    name: str

    @override
    def do(self, original: Program) -> Program:
        decl = find_element(self.target, original)
        if not isinstance(decl, elements.Declaration):
            raise BadEdit("Only a declaration may be renamed")
        decl.name, self.name = self.name, decl.name
        return original

    @override
    def undo(self, original: Program) -> Program:
        decl = find_element(self.target, original)
        assert isinstance(decl, elements.Declaration)
        decl.name, self.name = self.name, decl.name
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

        element.value, self.value = self.value, element.value
        # TODO: Update type here too?

        return original

    @override
    def undo(self, original: Program) -> Program:
        element = find_element(self.target, original)
        assert isinstance(element, elements.Constant)
        element.value, self.value = self.value, element.value
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

        element.op, self.value = elements.Operator(self.value), element.op
        return original

    @override
    def undo(self, original: Program) -> Program:
        element = find_element(self.target, original)
        assert isinstance(element, elements.BinaryOperator) or isinstance(
            element, elements.UnaryOperator
        )
        element.op, self.value = elements.Operator(self.value), element.op
        return original


class AddConduit(BaseModel, TransactionBase):
    discriminator: Literal["add_conduit"] = "add_conduit"
    source: str  # "input-QualifiedID-index"
    target: str  # "output-QualifiedID-index"
    added: QualifiedID | None = None
    previous: elements.Conduit | None = None

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
            self.previous = copy.deepcopy(existing_conduit)
            self.added = source_id.id[:-1] + [self.previous.id]
            # If a conduit already exists, we reset the source
            existing_conduit.input = source_id.id[-1]
            existing_conduit.index = source_id.index
            return original

        self.added = source_id.id[:-1] + [
            next_id(decl)
        ]  # ID should be generated by the system
        decl.conduits.append(
            elements.Conduit(
                id=self.added[-1],
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
        assert self.added is not None
        RemoveItem(target=self.added).do(original)
        if self.previous is not None:
            source_id = self._create_inout_id(self.source)
            decl = cast(
                elements.Function, find_element(source_id.id[:-1], original)
            )
            decl.conduits.append(self.previous)
        return original

    def _create_inout_id(self, description: str) -> InOutID:
        parts = description.split("-")
        if len(parts) != 3:
            raise BadEdit("Invalid source or target format for conduit")
        id = self.InOutID(
            id=make_qualified_id(parts[1]),
            index=int(parts[2]),
        )
        return id


class AddNode(BaseModel, TransactionBase):
    discriminator: Literal["add_node"] = "add_node"
    parent: QualifiedID
    new_type: Literal["Constant", "BinaryOperator", "UnaryOperator"]
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
        self._inserted = new_id
        return original

    @override
    def undo(self, original: Program) -> Program:
        assert self._inserted is not None
        return RemoveItem(target=self.parent + [self._inserted]).do(original)


class RemoveItem(BaseModel, TransactionBase):
    target: QualifiedID
    _removed_item: elements.Item | None = None
    _removed_conduits: list[elements.Conduit] = []

    @override
    def do(self, original: Program) -> Program:
        if len(self.target) < 1:
            raise BadEdit("Cannot remove the program itself")

        if len(self.target) == 1:
            # Removing a function from the program
            # Find and store the function being removed
            for func in original.declarations:
                if func.id == self.target[0]:
                    self._removed_item = func
                    break

            if self._removed_item is None:
                raise BadEdit(f"Function {self.target[0]} not found")

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
            # Find and store the item being removed
            self._removed_item = None

            # Check if it's a node
            for node in parent.nodes:
                if node.id == element_id:
                    self._removed_item = node
                    break

            # Check if it's a conduit
            if self._removed_item is None:
                for conduit in parent.conduits:
                    if conduit.id == element_id:
                        self._removed_item = conduit
                        break

            if self._removed_item is None:
                raise BadEdit(f"Element {element_id} not found")

            # Store conduits that will be removed due to connections
            self._removed_conduits = []
            for conduit in parent.conduits:
                if conduit.input == element_id or any(
                    cast(elements.Conduit.Output, out).target == element_id
                    for out in conduit.children
                ):
                    # Only store if it's not the item being directly removed
                    if conduit.id != element_id:
                        self._removed_conduits.append(conduit)

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
        if self._removed_item is None:
            raise BadEdit("Nothing to undo - no item was removed")

        if len(self.target) == 1:
            # Restoring a function to the program
            assert isinstance(self._removed_item, elements.Function)
            original.declarations.append(self._removed_item)
            return original

        parent_id = self.target[:-1]
        parent = find_item(parent_id, original)

        if isinstance(parent, elements.Function):
            # Restore the removed item
            if isinstance(self._removed_item, elements.Node):
                parent.nodes.append(self._removed_item)
            elif isinstance(self._removed_item, elements.Conduit):
                parent.conduits.append(self._removed_item)

            # Restore any conduits that were removed due to connections
            for conduit in self._removed_conduits:
                parent.conduits.append(conduit)
        else:
            raise BadEdit("Can only restore elements to functions")
        self._removed_conduits = []
        self._removed_item = None

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

from typing import Annotated, Literal, override

from pydantic import BaseModel, Field, PrivateAttr

from editor.models import Function, Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import Call, find_element
from editor.services.transaction.base import TransactionBase
from editor.utility.remove_connections import (
    ConnectionTarget,
    remove_inputs_to,
    remove_outputs_of,
)


class RenameCallArg(BaseModel):
    discriminator: Literal["rename_arg"] = "rename_arg"
    index: int
    name: str

    def do(self, node: Call, parent: Function) -> None:
        # Swap the argument and stored name so we can remember for undo
        node.arguments[self.index], self.name = (
            self.name,
            node.arguments[self.index],
        )

    def undo(self, node: Call, parent: Function) -> None:
        # `do` just swaps internally stored for the target name
        # so calling it here works because we stored the old name
        self.do(node, parent)


class AddCallArg(BaseModel):
    discriminator: Literal["add_arg"] = "add_arg"
    name: str
    _added_index: int = PrivateAttr(default=-1)

    def do(self, node: Call, parent: Function) -> None:
        self._added_index = len(node.arguments)
        node.arguments.append(self.name)
        node.location.height += 5

    def undo(self, node: Call, parent: Function) -> None:
        node.arguments.pop(self._added_index)
        node.location.height -= 5


class DeleteCallArg(BaseModel):
    discriminator: Literal["delete_arg"] = "delete_arg"
    index: int
    _deleted_name: str = PrivateAttr(default="")
    _removed_conduits: list[elements.Conduit] = PrivateAttr(
        default_factory=list
    )

    def do(self, node: Call, parent: Function) -> None:
        if self.index < 0 or self.index >= len(node.arguments):
            raise BadEdit(
                f"Call node has {len(node.arguments)} arguments, "
                f"cannot delete index {self.index}"
            )
        self._deleted_name = node.arguments.pop(self.index)
        self._removed_conduits = remove_inputs_to(
            ConnectionTarget(id=node.id, index=self.index), parent
        )
        node.location.height -= 5
        # TODO: Also remove conduits to this as well (could extract to standalone func)?

    def undo(self, node: Call, parent: Function) -> None:
        node.arguments.insert(self.index, self._deleted_name)
        node.location.height += 5


class ReorderCallArg(BaseModel):
    discriminator: Literal["reorder_arg"] = "reorder_arg"
    current: int
    destination: int

    def do(self, node: Call, parent: Function) -> None:
        n = len(node.arguments)
        if not (0 <= self.current < n) or not (0 <= self.destination < n):
            raise BadEdit(
                f"Call node has {n} arguments, "
                f"cannot reorder from {self.current} to {self.destination}"
            )
        node.arguments.insert(
            self.destination, node.arguments.pop(self.current)
        )

    def undo(self, node: Call, parent: Function) -> None:
        node.arguments.insert(
            self.current, node.arguments.pop(self.destination)
        )


class AddCallReturn(BaseModel):
    discriminator: Literal["add_return"] = "add_return"

    def do(self, node: Call, parent: Function) -> None:
        if node.returns:
            raise BadEdit("Call node already has a return value")
        node.returns = True

    def undo(self, node: Call, parent: Function) -> None:
        node.returns = False


class DeleteCallReturn(BaseModel):
    discriminator: Literal["delete_return"] = "delete_return"
    _removed_conduits: list[elements.Conduit] = PrivateAttr(
        default_factory=list
    )

    def do(self, node: Call, parent: Function) -> None:
        if not node.returns:
            raise BadEdit("Call node has no return value to delete")
        node.returns = False
        self._removed_conduits = remove_outputs_of(
            ConnectionTarget(id=node.id, index=0), parent
        )

    def undo(self, node: Call, parent: Function) -> None:
        node.returns = True
        parent.conduits.extend(self._removed_conduits)


CallCommand = Annotated[
    RenameCallArg
    | AddCallArg
    | DeleteCallArg
    | ReorderCallArg
    | AddCallReturn
    | DeleteCallReturn,
    Field(discriminator="discriminator"),
]


class EditCallNode(BaseModel, TransactionBase):
    discriminator: Literal["edit_call_node"] = "edit_call_node"
    target: QualifiedID
    command: CallCommand

    @override
    def do(self, original: Program) -> Program:
        parent = find_element(self.target[:-1], original)
        assert isinstance(parent, elements.Function)
        node = find_element(self.target, original)
        assert isinstance(node, elements.Call)
        self.command.do(node, parent)
        return original

    @override
    def undo(self, original: Program) -> Program:
        parent = find_element(self.target[:-1], original)
        assert isinstance(parent, elements.Function)
        node = find_element(self.target, original)
        assert isinstance(node, elements.Call)
        self.command.undo(node, parent)
        return original

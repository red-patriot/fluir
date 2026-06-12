from typing import Annotated, Literal, override

from pydantic import BaseModel, Field, PrivateAttr

from editor.models import Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import Call, find_element
from editor.services.transaction.base import TransactionBase


class RenameCallArg(BaseModel):
    discriminator: Literal["rename_arg"] = "rename_arg"
    index: int
    name: str

    def do(self, node: Call) -> None:
        # Swap the argument and stored name so we can remember for undo
        node.arguments[self.index], self.name = (
            self.name,
            node.arguments[self.index],
        )

    def undo(self, node: Call) -> None:
        # `do` just swaps internally stored for the target name
        # so calling it here works because we stored the old name
        self.do(node)


class AddCallArg(BaseModel):
    discriminator: Literal["add_arg"] = "add_arg"
    name: str
    _added_index: int = PrivateAttr(default=-1)

    def do(self, node: Call) -> None:
        self._added_index = len(node.arguments)
        node.arguments.append(self.name)
        node.location.height += 5

    def undo(self, node: Call) -> None:
        node.arguments.pop(self._added_index)
        node.location.height -= 5


class DeleteCallArg(BaseModel):
    discriminator: Literal["delete_arg"] = "delete_arg"
    index: int
    _deleted_name: str = PrivateAttr(default="")

    def do(self, node: Call) -> None:
        if self.index < 0 or self.index >= len(node.arguments):
            raise BadEdit(
                f"Call node has {len(node.arguments)} arguments, "
                f"cannot delete index {self.index}"
            )
        self._deleted_name = node.arguments.pop(self.index)
        node.location.height -= 5

    def undo(self, node: Call) -> None:
        node.arguments.insert(self.index, self._deleted_name)
        node.location.height += 5


class ReorderCallArg(BaseModel):
    discriminator: Literal["reorder_arg"] = "reorder_arg"
    current: int
    destination: int

    def do(self, node: Call) -> None:
        n = len(node.arguments)
        if not (0 <= self.current < n) or not (0 <= self.destination < n):
            raise BadEdit(
                f"Call node has {n} arguments, "
                f"cannot reorder from {self.current} to {self.destination}"
            )
        node.arguments.insert(
            self.destination, node.arguments.pop(self.current)
        )

    def undo(self, node: Call) -> None:
        node.arguments.insert(
            self.current, node.arguments.pop(self.destination)
        )


CallCommand = Annotated[
    RenameCallArg | AddCallArg | DeleteCallArg | ReorderCallArg,
    Field(discriminator="discriminator"),
]


class EditCallNode(BaseModel, TransactionBase):
    discriminator: Literal["edit_call_node"] = "edit_call_node"
    target: QualifiedID
    command: CallCommand

    @override
    def do(self, original: Program) -> Program:
        node = find_element(self.target, original)
        assert isinstance(node, elements.Call)
        self.command.do(node)
        return original

    @override
    def undo(self, original: Program) -> Program:
        node = find_element(self.target, original)
        assert isinstance(node, elements.Call)
        self.command.undo(node)
        return original

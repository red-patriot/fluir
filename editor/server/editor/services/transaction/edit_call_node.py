from typing import Annotated, Literal, override

from pydantic import BaseModel, Field, PrivateAttr

from editor.models import Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import find_element
from editor.services.transaction.base import TransactionBase


class RenameCallArg(BaseModel):
    discriminator: Literal["rename_arg"] = "rename_arg"
    index: int
    name: str

    def do(self, arguments: list[str]) -> None:
        # Swap the argument and stored name so we can remember for undo
        arguments[self.index], self.name = self.name, arguments[self.index]

    def undo(self, arguments: list[str]) -> None:
        # `do` just swaps internally stored for the target name
        # so calling it here works because we stored the old name
        self.do(arguments)


class AddCallArg(BaseModel):
    discriminator: Literal["add_arg"] = "add_arg"
    name: str
    _added_index: int = PrivateAttr(default=-1)

    def do(self, arguments: list[str]) -> None:
        self._added_index = len(arguments)
        arguments.append(self.name)

    def undo(self, arguments: list[str]) -> None:
        arguments.pop(self._added_index)


class DeleteCallArg(BaseModel):
    discriminator: Literal["delete_arg"] = "delete_arg"
    index: int
    _deleted_name: str = PrivateAttr(default="")

    def do(self, arguments: list[str]) -> None:
        if self.index < 0 or self.index >= len(arguments):
            raise BadEdit(
                f"Call node has {len(arguments)} arguments, "
                f"cannot delete index {self.index}"
            )
        self._deleted_name = arguments.pop(self.index)

    def undo(self, arguments: list[str]) -> None:
        arguments.insert(self.index, self._deleted_name)


CallCommand = Annotated[
    RenameCallArg | AddCallArg | DeleteCallArg,
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
        self.command.do(node.arguments)
        return original

    @override
    def undo(self, original: Program) -> Program:
        node = find_element(self.target, original)
        assert isinstance(node, elements.Call)
        self.command.undo(node.arguments)
        return original

from typing import Literal, override

from pydantic import BaseModel

from editor.models import Program, QualifiedID, elements
from editor.models.elements import find_element
from editor.services.transaction.base import TransactionBase


class RenameCallArg(BaseModel):
    discriminator: Literal["rename_arg"] = "rename_arg"
    index: int
    name: str


class EditCallNode(BaseModel, TransactionBase):
    discriminator: Literal["edit_call_node"] = "edit_call_node"
    target: QualifiedID
    command: RenameCallArg

    @override
    def do(self, original: Program) -> Program:
        node = find_element(self.target, original)
        assert isinstance(node, elements.Call)
        node.arguments[self.command.index], self.command.name = (
            self.command.name,
            node.arguments[self.command.index],
        )
        return original

    @override
    def undo(self, original: Program) -> Program:
        return self.do(original)

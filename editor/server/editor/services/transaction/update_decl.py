from typing import Annotated, Literal, override

from pydantic import BaseModel, Field

from editor.models import FlType, Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import find_item
from editor.services.transaction.base import TransactionBase


class UpdateFuncParam(BaseModel, TransactionBase):
    discriminator: Literal["update_func_param"] = "update_func_param"
    target: QualifiedID
    index: int
    type: FlType

    @override
    def do(self, original: Program) -> Program:
        function = find_item(self.target, original)
        if not isinstance(function, elements.Function):
            raise BadEdit(f"Function {self.target} not found")
        if self.index >= len(function.inputs):
            raise BadEdit(
                f"Function only has {len(function.inputs)} parameters, tried to edit index {self.index}"
            )

        old_type = function.inputs[self.index].flType
        assert old_type is not None
        function.inputs[self.index].flType = self.type
        self.type = old_type

        return original

    @override
    def undo(self, original: Program) -> Program:
        function = find_item(self.target, original)
        assert isinstance(function, elements.Function)
        old_type = function.inputs[self.index].flType
        assert old_type is not None
        function.inputs[self.index].flType = self.type
        self.type = old_type
        return original

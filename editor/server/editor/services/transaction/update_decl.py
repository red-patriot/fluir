from typing import Annotated, Literal, override

from pydantic import BaseModel, Field

from editor.models import FlType, Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import find_item
from editor.services.transaction.base import TransactionBase


class UpdateFuncParam(BaseModel, TransactionBase):
    class UpdateType(BaseModel):
        discriminator: Literal["type"] = "type"
        flType: FlType

    class UpdateName(BaseModel):
        discriminator: Literal["name"] = "name"
        name: str

    discriminator: Literal["update_func_param"] = "update_func_param"
    target: QualifiedID
    index: int
    cmd: UpdateType | UpdateName

    @override
    def do(self, original: Program) -> Program:
        function = find_item(self.target, original)
        if not isinstance(function, elements.Function):
            raise BadEdit(f"Function {self.target} not found")
        if self.index >= len(function.inputs):
            raise BadEdit(
                f"Function {function.name} only has {len(function.inputs)} parameters, tried to edit index {self.index}"
            )
        match self.cmd.discriminator:
            case "type":
                self._type(function)
            case "name":
                self._name(function)

        return original

    def _type(self, function: elements.Function) -> None:
        assert self.cmd.discriminator == "type"
        old_type = function.inputs[self.index].flType
        assert old_type is not None
        function.inputs[self.index].flType = self.cmd.flType
        self.cmd.flType = old_type

    def _name(self, function: elements.Function) -> None:
        assert self.cmd.discriminator == "name"
        old_name = function.inputs[self.index].name
        function.inputs[self.index].name = self.cmd.name
        self.cmd.name = old_name

    @override
    def undo(self, original: Program) -> Program:
        function = find_item(self.target, original)
        assert isinstance(function, elements.Function)
        match self.cmd.discriminator:
            case "type":
                self._type(function)
            case "name":
                self._name(function)
        return original


class UpdateFuncReturn(BaseModel, TransactionBase):
    discriminator: Literal["update_func_return"] = "update_func_return"
    target: QualifiedID
    type: FlType

    @override
    def do(self, original: Program) -> Program:
        function = find_item(self.target, original)
        if not isinstance(function, elements.Function):
            raise BadEdit(f"Function {self.target} not found")
        if 0 == len(function.outputs):
            raise BadEdit(f"Function {function.name} nas no return value")

        old_type = function.outputs[0].flType
        assert old_type is not None
        function.outputs[0].flType = self.type
        self.type = old_type

        return original

    @override
    def undo(self, original: Program) -> Program:
        function = find_item(self.target, original)
        assert isinstance(function, elements.Function)
        old_type = function.outputs[0].flType
        assert old_type is not None
        function.outputs[0].flType = self.type
        self.type = old_type
        return original

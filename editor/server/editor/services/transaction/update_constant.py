from typing import Literal, override

from pydantic import BaseModel

from editor.models import Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import find_element
from editor.services.transaction.base import TransactionBase


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

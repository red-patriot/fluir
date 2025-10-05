from typing import Literal, override

from pydantic import BaseModel

from editor.models import Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import find_element
from editor.services.transaction.base import TransactionBase


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

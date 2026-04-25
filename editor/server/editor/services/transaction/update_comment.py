from typing import Literal, override

from pydantic import BaseModel

from editor.models import Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import find_element
from editor.services.transaction.base import TransactionBase


class UpdateComment(BaseModel, TransactionBase):
    discriminator: Literal["update_comment"] = "update_comment"
    target: QualifiedID
    data: str

    @override
    def do(self, original: Program) -> Program:
        element = find_element(self.target, original)
        if not isinstance(element, elements.Comment):
            raise BadEdit("Can only perform this update on a comment")

        element.data, self.data = self.data, element.data

        return original

    @override
    def undo(self, original: Program) -> Program:
        element = find_element(self.target, original)
        assert isinstance(element, elements.Comment)
        element.data, self.data = self.data, element.data
        return original

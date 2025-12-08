from dataclasses import dataclass
from typing import Literal, override

from pydantic import BaseModel

from editor.models import Program, QualifiedID
from editor.models.edit_errors import BadEdit
from editor.models.elements import find_element
from editor.services.transaction.base import TransactionBase


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

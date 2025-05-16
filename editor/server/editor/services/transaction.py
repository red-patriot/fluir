from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import Literal, override

from pydantic import BaseModel

from editor.models import Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import find_element


class TransactionBase(ABC):
    """A transaction that can be accepted from the client to edit a program"""

    @abstractmethod
    def do(self, original: Program) -> Program:
        """Apply an operation to the given program and return the resulting program"""

    @abstractmethod
    def undo(self, original: Program) -> Program:
        """Undoes the operation performed in `do`"""


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
        new_x = element.location.x + self.x
        new_y = element.location.y + self.y
        if len(self.target) > 1:
            xlim, ylim = self._get_limits(original)
            if (
                new_x < xlim.lower
                or new_y < ylim.lower
                or (new_x + element.location.width) > xlim.upper
                or (new_y + element.location.height) > ylim.upper
            ):
                raise BadEdit("Element cannot be moved out of bounds")
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
        return original


class UpdateConstant(BaseModel, TransactionBase):
    discriminator: Literal["update_constant"] = "update_constant"
    target: QualifiedID
    value: str

    @override
    def do(self, original: Program) -> Program:
        element = find_element(self.target, original)
        if not isinstance(element, elements.Constant):
            raise BadEdit("Can only perform this update on a constant")

        element.value = self.value
        # TODO: Update type here too?

        return original

    @override
    def undo(self, original: Program) -> Program:
        return original


type EditTransaction = MoveElement | UpdateConstant

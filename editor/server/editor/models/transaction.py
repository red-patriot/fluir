from abc import ABC, abstractmethod
from typing import override

from pydantic import BaseModel

from editor.models import Program, QualifiedID
from editor.models.edit_errors import BadEdit
from editor.models.elements import Declaration, find_element


class EditTransaction(ABC):
    """A transaction that can be accepted from the client to edit a program"""

    @abstractmethod
    def do(self, original: Program) -> Program:
        """Apply an operation to the given program and return the resulting program"""

    @abstractmethod
    def undo(self, original: Program) -> Program:
        """Undoes the operation performed in `do`"""


class MoveElement(BaseModel, EditTransaction):
    target: QualifiedID
    x: int
    y: int

    @override
    def do(self, original: Program) -> Program:
        element = find_element(self.target, original)
        element.location.x = self.x
        element.location.y = self.y

        return original

    @override
    def undo(self, original: Program) -> Program:
        return original

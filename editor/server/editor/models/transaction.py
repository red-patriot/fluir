from abc import ABC, abstractmethod
from typing import override

from pydantic import BaseModel

from editor.models import IDType, Program


class EditTransaction(ABC):
    """A transaction that can be accepted from the client to edit a program"""

    @abstractmethod
    def do(self, original: Program) -> Program:
        """Apply an operation to the given program and return the resulting program"""

    @abstractmethod
    def undo(self, original: Program) -> Program:
        """Undoes the operation performed in `do`"""


class MoveElement(BaseModel, EditTransaction):
    target: list[IDType]
    x: int
    y: int

    @override
    def do(self, original: Program) -> Program:
        element = None
        for decl in original.declarations:
            if decl.id == self.target[0]:
                element = decl
                break
        assert element is not None
        element.location.x = self.x
        element.location.y = self.y
        return original

    @override
    def undo(self, original: Program) -> Program:
        return original

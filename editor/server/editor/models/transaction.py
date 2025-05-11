from abc import ABC, abstractmethod

from pydantic.dataclasses import dataclass

from editor.models import Program


class EditTransaction(ABC):
    """A transaction that can be accepted from the client to edit a program"""

    @abstractmethod
    def do(self, original: Program) -> Program:
        """Apply an operation to the given program and return the resulting program"""

    @abstractmethod
    def undo(self, original: Program) -> Program:
        """Undoes the operation performed in `do`"""

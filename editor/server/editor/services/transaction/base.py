from abc import ABC, abstractmethod

from editor.models import Program
from editor.services.intelligence import IntelligenceReadInterface


class TransactionBase(ABC):
    """A transaction that can be accepted from the client to edit a program"""

    def resolve(self, intelligence: IntelligenceReadInterface) -> None:
        """
        Resolve data given an intelligence source. Optional to implement.
        Always called before `do`
        """
        pass

    @abstractmethod
    def do(self, original: Program) -> Program:
        """Apply an operation to the given program and return the resulting program"""

    @abstractmethod
    def undo(self, original: Program) -> Program:
        """Undoes the operation performed in `do`"""

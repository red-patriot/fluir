from abc import ABC, abstractmethod
from pathlib import Path

from editor.models import elements
from editor.models.elements import FlType, Program
from editor.models.id import QualifiedID
from editor.models.intelligence import FunctionSignature
from editor.models.lsp.completion import Completion, Kind


class IntelligenceReadInterface(ABC):
    """The interface to read intelligence about a program"""

    @abstractmethod
    def get_completions(
        self, block_id: QualifiedID, path: Path
    ) -> list[Completion]:
        """Returns a list of available completions at the given location in the given path"""

    @abstractmethod
    def get_operators(
        self, operator_id: QualifiedID, arity: int, path: Path
    ) -> list[elements.Operator]:
        """Returns a list of available operators at the given location in the given path"""

    @abstractmethod
    def get_types(self, item_id: QualifiedID, path: Path) -> list[str]:
        """Returns a list of types visible from the given location in the given path"""

    @abstractmethod
    def get_function_signature(
        self, func_name: str, path: Path
    ) -> FunctionSignature | None:
        """Return the signature of the given function at the given location in the given program path,
        or None if the function does not exist"""

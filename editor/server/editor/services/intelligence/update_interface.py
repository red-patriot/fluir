from abc import ABC, abstractmethod
from pathlib import Path

from editor.models import elements


class IntelligenceUpdateInterface(ABC):
    """The interface to update intelligence about the program"""

    @abstractmethod
    def add_module(self, program: elements.Program, path: Path) -> None:
        """Adds a module"""

    @abstractmethod
    def remove_module(self, path: Path) -> None:
        """Removes a module"""

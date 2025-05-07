from abc import ABC, abstractmethod
from pathlib import Path

from editor.models.elements import Program


class FileManager(ABC):
    """Handles CRUD operations to and parsing of Fluir source files."""

    @abstractmethod
    def parseStr(self, source: bytes) -> Program:
        """Parses the given source code into a Fluir program"""

    @abstractmethod
    def parseFile(self, file: Path) -> Program:
        """Opens the given file and parses its contents into a Fluir Program"""

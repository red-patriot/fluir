from pathlib import Path

from editor.models.elements import Program
from editor.models.id import QualifiedID
from editor.models.lsp.completion import Completion, Kind


class IntelligenceService:
    """A Service to provide intelligence for Fluir modules"""

    def __init__(self) -> None:
        pass

    # TODO: Add other capabilities here

    def add_module(self, program: Program, path: Path) -> None:
        pass

    def remove_module(self, path: Path) -> None:
        pass

    def get_completions(
        self, block_id: QualifiedID, path: Path
    ) -> list[Completion]:
        """Return a list of available completions at the given location in the given program path"""
        return self._builtin_operators()

    def _builtin_operators(self) -> list[Completion]:
        # TODO: Don't just hardcode things here...
        return [
            Completion(short_name="binary +", kind=Kind.OPERATOR),
            Completion(short_name="binary -", kind=Kind.OPERATOR),
            Completion(short_name="binary *", kind=Kind.OPERATOR),
            Completion(short_name="binary /", kind=Kind.OPERATOR),
            Completion(short_name="unary +", kind=Kind.OPERATOR),
            Completion(short_name="unary -", kind=Kind.OPERATOR),
            Completion(short_name="unary ++", kind=Kind.OPERATOR),
            Completion(short_name="unary --", kind=Kind.OPERATOR),
        ]

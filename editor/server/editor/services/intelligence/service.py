from pathlib import Path

from editor.models.elements import FlType, Program
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
        if len(block_id) == 0:
            # Top level provides definition completions
            return self._toplevel_options()
        return self._builtin_operators() + self._constants()

    def get_types(self, block_id: QualifiedID, path: Path) -> list[str]:
        """Return a list of types visible at the given location in the given program path"""
        return self._builtin_types()

    def _toplevel_options(self) -> list[Completion]:
        return [
            Completion(
                short_name="function",
                kind=Kind.FUNCTION_DEF,
                description="Define a new function here",
            ),
        ]

    def _builtin_operators(self) -> list[Completion]:
        # TODO: Don't just hardcode things here...
        return [
            Completion(short_name="+ (binary)", kind=Kind.OPERATOR),
            Completion(short_name="- (binary)", kind=Kind.OPERATOR),
            Completion(short_name="* (binary)", kind=Kind.OPERATOR),
            Completion(short_name="/ (binary)", kind=Kind.OPERATOR),
            Completion(short_name="+ (unary)", kind=Kind.OPERATOR),
            Completion(short_name="- (unary)", kind=Kind.OPERATOR),
            Completion(short_name="++ (unary)", kind=Kind.OPERATOR),
            Completion(short_name="-- (unary)", kind=Kind.OPERATOR),
        ]

    def _constants(self) -> list[Completion]:
        return [
            Completion(short_name=t.value, kind=Kind.CONSTANT) for t in FlType
        ]

    def _builtin_types(self) -> list[str]:
        return [t for t in FlType]

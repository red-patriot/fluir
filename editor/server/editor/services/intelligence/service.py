from pathlib import Path

from editor.models import elements
from editor.models.elements import FlType, Program
from editor.models.id import QualifiedID
from editor.models.intelligence import FunctionSignature
from editor.models.lsp.completion import Completion, Kind


class IntelligenceService:
    """A Service to provide intelligence for Fluir modules"""

    def __init__(self) -> None:
        self._functions: dict[Path, dict[str, FunctionSignature]] = {}

    # TODO: Add other capabilities here

    def add_module(self, program: Program, path: Path) -> None:
        self._functions[path] = {
            decl.name: self._get_sig(decl) for decl in program.declarations
        }

    @staticmethod
    def _get_sig(decl: elements.Function) -> FunctionSignature:
        return FunctionSignature(
            inputs=[str(i.flType) for i in decl.inputs],
            # TODO: handle multiple returns
            output=str(decl.outputs[0].flType)
            if len(decl.outputs) > 0
            else None,
        )

    def remove_module(self, path: Path) -> None:
        self._functions.pop(path, None)

    def get_completions(
        self, block_id: QualifiedID, path: Path
    ) -> list[Completion]:
        """Return a list of available completions at the given location in the given program path"""
        if path not in self._functions:
            return []
        if len(block_id) == 0:
            # Top level provides definition completions
            return self._toplevel_options() + self._annotations()
        return (
            self._builtin_operators()
            + self._constants()
            + self._function_completions(path)
            + self._annotations()
        )

    def get_types(self, block_id: QualifiedID, path: Path) -> list[str]:
        """Return a list of types visible at the given location in the given program path"""
        if path not in self._functions:
            return []
        return self._builtin_types()

    def get_function_signature(
        self, func_name: str, path: Path
    ) -> FunctionSignature | None:
        """Return the signature of the given function at the given location in the given program path,
        or None if the function does not exist"""
        signatures: dict[str, FunctionSignature] = self._functions.get(
            path, dict()
        )
        if func_name in signatures:
            return signatures[func_name]
        return None

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

    def _function_completions(self, path: Path) -> list[Completion]:
        # TODO: Add documentation when that is implemented
        return [
            Completion(short_name=name, kind=Kind.CALL)
            for name in self._functions.get(path, dict()).keys()
        ] + self._builtin_functions()

    def _constants(self) -> list[Completion]:
        return [
            Completion(short_name=t.value, kind=Kind.CONSTANT) for t in FlType
        ]

    def _builtin_types(self) -> list[str]:
        return [t for t in FlType]

    def _annotations(self) -> list[Completion]:
        return [
            Completion(short_name="comment", kind=Kind.COMMENT),
            Completion(short_name="//", kind=Kind.COMMENT),
        ]

    def _builtin_functions(self) -> list[Completion]:
        return [
            Completion(short_name="print", kind=Kind.CALL),
            # TODO: add other builtin functions here
        ]

from pathlib import Path

import pytest

from editor.models import Function, Program
from editor.models.elements import FlType
from editor.models.lsp.completion import Completion, Kind
from editor.services.intelligence import IntelligenceService

# TODO: Add a test like this to check complete type info in a program
# @pytest.mark.parametrize(
#     "program",
#     get_test_programs("type_check", extension=".fl"),
#     ids=file_path_name,
# )
# def test_type_check(program: Path) -> None:
#     info_golden_file = get_golden_file_path(get_relative_path(program), ".symbols.json")
#     diagnostics_golden_file = get_golden_file_path(get_relative_path(program), ".diags.json")
#     file_manager = XMLFileManager()
#     parsed = file_manager.parseFile(program)
#
#     uut = IntelligenceService()
#     uut.add_module(parsed, program)
#
#     expected_info = json.loads(info_golden_file.read_text())
#     expected_diagnostics = json.loads(diagnostics_golden_file.read_text())
#
#     assert uut.get_all_node_info(program) == expected_info
#     assert uut.get_diagnostics(program) == expected_diagnostics


@pytest.mark.parametrize(
    "expected",
    [
        Completion(short_name="+ (binary)", kind=Kind.OPERATOR),
        Completion(short_name="- (binary)", kind=Kind.OPERATOR),
        Completion(short_name="* (binary)", kind=Kind.OPERATOR),
        Completion(short_name="/ (binary)", kind=Kind.OPERATOR),
        Completion(short_name="+ (unary)", kind=Kind.OPERATOR),
        Completion(short_name="- (unary)", kind=Kind.OPERATOR),
        Completion(short_name="++ (unary)", kind=Kind.OPERATOR),
        Completion(short_name="-- (unary)", kind=Kind.OPERATOR),
    ],
)
def test_operator_completions(expected: Completion) -> None:
    """Tests that completions provide builtin operators"""
    path = Path("fake/path/to/program.fl")

    program = Program(  # Empty function
        declarations=[
            Function(
                name="main",
                id=1,
            )
        ]
    )

    uut = IntelligenceService()
    uut.add_module(program, path)
    actual = uut.get_completions([1], path)

    assert expected in actual


@pytest.mark.parametrize(
    "expected",
    [Completion(short_name=t.value, kind=Kind.CONSTANT) for t in FlType],
)
def test_constant_completions(expected: Completion) -> None:
    """Tests that completions provide constants for all FlType values"""
    path = Path("fake/path/to/program.fl")

    program = Program(  # Empty function
        declarations=[
            Function(
                name="main",
                id=1,
            )
        ]
    )

    uut = IntelligenceService()
    uut.add_module(program, path)
    actual = uut.get_completions([1], path)

    assert expected in actual


@pytest.mark.parametrize(
    "expected",
    [t.value for t in FlType],
)
def test_builtin_types(expected: str) -> None:
    """Tests that get_types provides all builtin FlType values"""
    path = Path("fake/path/to/program.fl")

    program = Program(
        declarations=[
            Function(
                name="main",
                id=1,
            )
        ]
    )

    uut = IntelligenceService()
    uut.add_module(program, path)
    actual = uut.get_types([1], path)

    assert expected in actual


def test_function_def_completion_at_top_level() -> None:
    expected = Completion(
        short_name="function",
        kind=Kind.FUNCTION_DEF,
        description="Define a new function here",
    )
    path = Path("fake/path/to/program.fl")

    program = Program(  # Empty function
        declarations=[
            Function(
                name="main",
                id=1,
            )
        ]
    )

    uut = IntelligenceService()
    uut.add_module(program, path)

    actual = uut.get_completions([], path)

    assert expected in actual

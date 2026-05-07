from pathlib import Path

import pytest

from editor.models import Function, Program, elements
from editor.models.elements import FlType
from editor.models.intelligence import FunctionSignature
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


@pytest.fixture
def program() -> Program:
    return Program(
        declarations=[
            Function(
                name=f"func_{letter}",
                id=i,
                inputs=[
                    elements.Parameter(flType=FlType("I32")),
                    elements.Parameter(flType=FlType("I32")),
                ],
                outputs=[elements.Return(flType=FlType("I32"))],
            )
            for i, letter in enumerate("abcde", start=1)
        ]
    )


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
def test_operator_completions(expected: Completion, program: Program) -> None:
    """Tests that completions provide builtin operators"""
    path = Path("fake/path/to/program.fl")

    uut = IntelligenceService()
    uut.add_module(program, path)
    actual = uut.get_completions([1], path)

    assert expected in actual


@pytest.mark.parametrize(
    "expected",
    [Completion(short_name=t.value, kind=Kind.CONSTANT) for t in FlType],
)
def test_constant_completions(expected: Completion, program: Program) -> None:
    """Tests that completions provide constants for all FlType values"""
    path = Path("fake/path/to/program.fl")

    uut = IntelligenceService()
    uut.add_module(program, path)
    actual = uut.get_completions([1], path)

    assert expected in actual


@pytest.mark.parametrize(
    "expected",
    [t.value for t in FlType],
)
def test_builtin_types(expected: str, program: Program) -> None:
    """Tests that get_types provides all builtin FlType values"""
    path = Path("fake/path/to/program.fl")

    uut = IntelligenceService()
    uut.add_module(program, path)
    actual = uut.get_types([1], path)

    assert expected in actual


def test_function_def_completion_at_top_level(program: Program) -> None:
    expected = Completion(
        short_name="function",
        kind=Kind.FUNCTION_DEF,
        description="Define a new function here",
    )
    path = Path("fake/path/to/program.fl")

    uut = IntelligenceService()
    uut.add_module(program, path)

    actual = uut.get_completions([], path)

    assert expected in actual


def test_completion_includes_other_functions(program: Program) -> None:
    """Tests that the completion includes other functions visible from the provided one"""
    path = Path("fake/path/to/program.fl")
    expecteds = ("func_b", "func_c", "func_d", "func_e")

    uut = IntelligenceService()
    uut.add_module(program, path)

    actual = uut.get_completions([1], path)

    for func_name in expecteds:
        expected = Completion(short_name=func_name, kind=Kind.CALL)
        assert expected in actual


def test_get_completions_returns_empty_for_unknown_path() -> None:
    """Tests that completions are empty when the path has not been added"""
    uut = IntelligenceService()

    assert uut.get_completions([], Path("unknown.fl")) == []
    assert uut.get_completions([1], Path("unknown.fl")) == []


def test_get_types_returns_empty_for_unknown_path() -> None:
    """Tests that types are empty when the path has not been added"""
    uut = IntelligenceService()

    assert uut.get_types([1], Path("unknown.fl")) == []


def test_remove_module_clears_completions(program: Program) -> None:
    """Tests that remove_module drops a previously added path"""
    path = Path("fake/path/to/program.fl")

    uut = IntelligenceService()
    uut.add_module(program, path)
    uut.remove_module(path)

    assert uut.get_completions([1], path) == []
    assert uut.get_types([1], path) == []


def test_remove_module_is_noop_for_unknown_path() -> None:
    """Tests that remove_module does not raise when the path is unknown"""
    uut = IntelligenceService()

    uut.remove_module(Path("never/added.fl"))


def test_get_function_signature_returns_correct_data(program: Program) -> None:
    """Tests that get_function_signature returns the correct signature"""
    path = Path("fake/path/to/program.fl")
    expected = FunctionSignature(inputs=["I32", "I32"], output="I32")

    uut = IntelligenceService()
    uut.add_module(program, path)

    actual = uut.get_function_signature("func_a", path)

    assert expected == actual

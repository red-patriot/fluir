import copy

import pytest

from editor.models import Program, elements
from editor.models.edit_errors import BadEdit
from editor.services.module_editor import ModuleEditor
from editor.services.transaction import MoveElement


@pytest.fixture
def basic_program() -> Program:
    return Program(
        [
            elements.Function(
                name="foo",
                location=elements.Location(10, 10, 3, 100, 100),
                id=1,
            ),
            elements.Function(
                name="bar",
                location=elements.Location(210, 10, 2, 100, 100),
                id=2,
                body=[
                    elements.BinaryOperator(
                        id=1,
                        location=elements.Location(15, 2, 1, 5, 5),
                        op=elements.Operator.PLUS,
                        lhs=2,
                        rhs=3,
                    ),
                    elements.Constant(
                        id=2,
                        location=elements.Location(2, 2, 1, 5, 5),
                        value=3.0,
                    ),
                    elements.Constant(
                        id=3,
                        location=elements.Location(2, 12, 1, 5, 5),
                        value=2.0,
                    ),
                ],
            ),
        ]
    )


@pytest.fixture
def editor(basic_program: Program) -> ModuleEditor:
    editor = ModuleEditor()
    editor.open_module(copy.deepcopy(basic_program))
    return editor


def test_move_function(basic_program: Program, editor: ModuleEditor) -> None:
    expected = copy.deepcopy(basic_program)
    expected.declarations[0].location = elements.Location(12, 47, 3, 100, 100)

    uut = MoveElement(target=[1], x=12, y=47)

    editor.edit(uut)
    actual = editor.get()

    assert expected == actual


def test_move_node(basic_program: Program, editor: ModuleEditor) -> None:
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].body[2].location = elements.Location(
        4, 78, 1, 5, 5
    )

    uut = MoveElement(target=[2, 3], x=4, y=78)

    editor.edit(uut)
    actual = editor.get()

    assert expected == actual


@pytest.mark.parametrize(
    "x, y",
    [(3, 100), (100, 13), (98, 12), (96, 53), (-3, 48), (0, -12), (-10, -10)],
)
def test_move_node_raises_if_out_of_function(
    basic_program: Program,
    editor: ModuleEditor,
    x: int,
    y: int,
) -> None:
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].body[2].location = elements.Location(
        4, 78, 1, 5, 5
    )

    uut = MoveElement(target=[2, 3], x=x, y=y)

    with pytest.raises(BadEdit):
        editor.edit(uut)

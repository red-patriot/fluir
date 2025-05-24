import copy

import pytest

from editor.models import FlType, Program, elements
from editor.models.edit_errors import BadEdit
from editor.services.module_editor import ModuleEditor
from editor.services.transaction import MoveElement, UpdateConstant


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
                nodes=[
                    elements.BinaryOperator(
                        id=1,
                        location=elements.Location(15, 2, 1, 5, 5),
                        op=elements.Operator.PLUS,
                    ),
                    elements.Constant(
                        id=2,
                        location=elements.Location(2, 2, 1, 5, 5),
                        value="3.0",
                        flType=FlType.FLOATING_POINT,
                    ),
                    elements.Constant(
                        id=3,
                        location=elements.Location(2, 12, 1, 5, 5),
                        value="2.0",
                        flType=FlType.FLOATING_POINT,
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
    expected.declarations[0].location = elements.Location(22, 57, 3, 100, 100)

    uut = MoveElement(target=[1], x=12, y=47)

    editor.edit(uut)
    actual = editor.get()

    assert expected == actual


def test_move_node(basic_program: Program, editor: ModuleEditor) -> None:
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].nodes[2].location = elements.Location(
        6, 90, 1, 5, 5
    )

    uut = MoveElement(target=[2, 3], x=4, y=78)

    editor.edit(uut)
    actual = editor.get()

    assert expected == actual


@pytest.mark.parametrize(
    "x, y",
    [(3, 88), (98, 13), (94, 12), (53, 84), (-3, 48), (0, -13), (-3, -13)],
)
# limit=elements.Location(210, 10, 2, 100, 100)
# element=elements.Location(2, 12, 1, 5, 5)
def test_move_node_raises_if_out_of_function(
    basic_program: Program,
    editor: ModuleEditor,
    x: int,
    y: int,
) -> None:
    uut = MoveElement(target=[2, 3], x=x, y=y)

    with pytest.raises(BadEdit):
        editor.edit(uut)


def test_edit_constant(basic_program: Program, editor: ModuleEditor) -> None:
    expected = copy.deepcopy(basic_program)
    assert isinstance(expected.declarations[1].nodes[1], elements.Constant)
    expected.declarations[1].nodes[1].value = "-5.67"

    uut = UpdateConstant(target=[2, 2], value="-5.67")
    editor.edit(uut)

    actual = editor.get()

    assert expected == actual


def test_edit_constant_raises_if_not_constant(editor: ModuleEditor) -> None:
    uut = UpdateConstant(target=[2, 1], value="-5.67")
    with pytest.raises(BadEdit):
        editor.edit(uut)

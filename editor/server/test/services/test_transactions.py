import copy

import pytest

from editor.models import FlType, Program, elements
from editor.models.edit_errors import BadEdit
from editor.services.module_editor import ModuleEditor
from editor.services.transaction import (
    AddConduit,
    AddNode,
    MoveElement,
    RemoveItem,
    RenameDeclaration,
    UpdateConstant,
    UpdateOperator,
)


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
                conduits=[
                    elements.Conduit(
                        id=5,
                        input=2,
                        children=[elements.Conduit.Output(target=1, index=1)],
                    )
                ],
            ),
            elements.Function(
                name="qux",
                location=elements.Location(210, 10, 2, 100, 100),
                id=3,
                nodes=[
                    elements.UnaryOperator(
                        id=2,
                        location=elements.Location(15, 2, 1, 5, 5),
                        op=elements.Operator.MINUS,
                    ),
                    elements.Constant(
                        id=3,
                        location=elements.Location(2, 12, 1, 5, 5),
                        value="2.0",
                        flType=FlType.FLOATING_POINT,
                    ),
                ],
                conduits=[
                    elements.Conduit(
                        id=5,
                        input=3,
                        children=[elements.Conduit.Output(target=2, index=0)],
                    )
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

    uut = MoveElement(target=[1], x=22, y=57)

    editor.edit(uut)
    actual = editor.get()

    assert expected == actual


def test_move_node(basic_program: Program, editor: ModuleEditor) -> None:
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].nodes[2].location = elements.Location(
        6, 90, 1, 5, 5
    )

    uut = MoveElement(target=[2, 3], x=6, y=90)

    editor.edit(uut)
    actual = editor.get()

    assert expected == actual


@pytest.mark.parametrize(
    "x, y",
    [(3, 101), (102, 13), (96, 12), (53, 97), (-3, 48), (0, -13), (-3, -13)],
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


def test_rename_function(basic_program: Program, editor: ModuleEditor) -> None:
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].name = "baz"

    uut = RenameDeclaration(target=[2], name="baz")

    editor.edit(uut)
    actual = editor.get()

    assert expected == actual


def test_rename_declaration_raises_if_target_is_not_a_decl(
    basic_program: Program, editor: ModuleEditor
) -> None:
    uut = RenameDeclaration(target=[2, 1], name="baz")

    with pytest.raises(BadEdit, match="Only a declaration may be renamed"):
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


@pytest.mark.parametrize(
    "op",
    ["+", "-", "*", "/"],
)
def test_update_binary_operator(
    basic_program: Program, editor: ModuleEditor, op: str
) -> None:
    expected = copy.deepcopy(basic_program)
    assert isinstance(
        expected.declarations[1].nodes[0], elements.BinaryOperator
    )
    expected.declarations[1].nodes[0].op = elements.Operator(op)

    uut = UpdateOperator(target=[2, 1], value=op)
    editor.edit(uut)

    actual = editor.get()

    assert expected == actual


@pytest.mark.parametrize(
    "op",
    ["+", "-", "*", "/"],
)
def test_update_unary_operator(
    basic_program: Program, editor: ModuleEditor, op: str
) -> None:
    expected = copy.deepcopy(basic_program)
    assert isinstance(expected.declarations[2].nodes[0], elements.UnaryOperator)
    expected.declarations[2].nodes[0].op = elements.Operator(op)

    uut = UpdateOperator(target=[3, 2], value=op)
    editor.edit(uut)

    actual = editor.get()

    assert expected == actual


def test_update_operator_raises_if_not_operator(editor: ModuleEditor) -> None:
    uut = UpdateOperator(target=[2, 2], value=elements.Operator.MINUS)
    with pytest.raises(
        BadEdit, match="Can only perform this update on an operator"
    ):
        editor.edit(uut)


def test_add_conduit_no_conflicts(
    basic_program: Program, editor: ModuleEditor
) -> None:
    expected = copy.deepcopy(basic_program)
    new_conduit = elements.Conduit(
        id=6,
        input=2,
        index=0,
        children=[
            elements.Conduit.Output(target=1, index=0),
        ],
    )
    expected.declarations[1].conduits.append(new_conduit)

    uut = AddConduit(target="input-2:1-0", source="output-2:2-0")
    editor.edit(uut)

    actual = editor.get()

    assert expected == actual


def test_add_conduit_removes_duplicate_targets(
    basic_program: Program, editor: ModuleEditor
) -> None:
    expected = copy.deepcopy(basic_program)
    new_conduit = elements.Conduit(
        id=4,
        input=2,
        index=0,
        children=[
            elements.Conduit.Output(target=1, index=0),
        ],
    )
    expected.declarations[1].conduits.append(new_conduit)
    existing_conduit = elements.Conduit(
        id=4,
        input=3,
        index=0,
        children=[
            elements.Conduit.Output(target=1, index=0),
        ],
    )
    # This conduit targets the same output as the new conduit,
    # so it should be overwritten
    program = editor.get()
    assert program is not None
    program.declarations[1].conduits.append(existing_conduit)

    uut = AddConduit(target="input-2:1-0", source="output-2:2-0")
    editor.edit(uut)

    actual = editor.get()

    assert expected == actual


@pytest.mark.parametrize(
    "expected_node, input",
    [
        (
            elements.Constant(
                id=6,
                location=elements.Location(2, 2, 0, 5, 5),
                value="0.0",
                flType=FlType.FLOATING_POINT,
            ),
            AddNode(
                parent=[2],
                new_type="Constant",
                new_location=elements.Location(
                    x=2, y=2, z=0, width=5, height=5
                ),
            ),
        ),
        (
            elements.BinaryOperator(
                id=6,
                location=elements.Location(15, 2, 1, 5, 5),
                op=elements.Operator.UNKNOWN,
            ),
            AddNode(
                parent=[2],
                new_type="BinaryOperator",
                new_location=elements.Location(15, 2, 1, 5, 5),
            ),
        ),
        (
            elements.UnaryOperator(
                id=6,
                location=elements.Location(2, 7, 0, 7, 7),
                op=elements.Operator.UNKNOWN,
            ),
            AddNode(
                parent=[2],
                new_type="UnaryOperator",
                new_location=elements.Location(2, 7, 0, 7, 7),
            ),
        ),
    ],
)
def test_add_node(
    expected_node: elements.Node,
    input: AddNode,
    basic_program: Program,
    editor: ModuleEditor,
) -> None:
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].nodes.append(expected_node)

    editor.edit(input)
    actual = editor.get()

    assert expected == actual


def test_remove_node(basic_program: Program, editor: ModuleEditor) -> None:
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].nodes.pop(2)

    uut = RemoveItem(target=[2, 3])
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual


def test_remove_node_with_conduits(
    basic_program: Program, editor: ModuleEditor
) -> None:
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].nodes.pop(1)
    expected.declarations[1].conduits.pop()

    uut = RemoveItem(target=[2, 2])
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual


def test_remove_function(basic_program: Program, editor: ModuleEditor) -> None:
    expected = copy.deepcopy(basic_program)
    expected.declarations.pop(0)

    uut = RemoveItem(target=[1])
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual


def test_remove_conduit(basic_program: Program, editor: ModuleEditor) -> None:
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].conduits.pop(0)

    uut = RemoveItem(target=[2, 5])
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual

import copy
from pathlib import Path
from typing import cast

import pytest
from pydantic import ValidationError

from editor.models import FlType, Program, elements
from editor.models.edit_errors import BadEdit
from editor.services.intelligence import IntelligenceReadInterface
from editor.services.module_editor import ModuleEditor
from editor.services.transaction import (
    AddNode,
    ConstantParams,
    FunctionParams,
    OperatorParams,
)
from editor.services.transaction.add_node import CallParams


@pytest.mark.parametrize(
    "expected_node, input",
    [
        (
            elements.Constant(
                id=6,
                location=elements.Location(2, 2, 0, 5, 5),
                value="0.0",
                flType=FlType.F64,
            ),
            AddNode(
                parent=[2],
                new_location=elements.Location(
                    x=2, y=2, z=0, width=5, height=5
                ),
                params=ConstantParams(type=FlType.F64),
            ),
        ),
        *(
            (
                elements.Constant(
                    id=6,
                    location=elements.Location(2, 2, 0, 5, 5),
                    value="0",
                    flType=FlType(elem),
                ),
                AddNode(
                    parent=[2],
                    new_location=elements.Location(
                        x=2, y=2, z=0, width=5, height=5
                    ),
                    params=ConstantParams(type=FlType(elem)),
                ),
            )
            for elem in ["I8", "I16", "I32", "I64", "U8", "U16", "U32", "U64"]
        ),
        (
            elements.BinaryOperator(
                id=6,
                location=elements.Location(15, 2, 1, 5, 5),
                op=elements.Operator.PLUS,
            ),
            AddNode(
                parent=[2],
                new_location=elements.Location(15, 2, 1, 5, 5),
                params=OperatorParams(arity="binary"),
            ),
        ),
        (
            elements.UnaryOperator(
                id=6,
                location=elements.Location(2, 7, 0, 7, 7),
                op=elements.Operator.PLUS,
            ),
            AddNode(
                parent=[2],
                new_location=elements.Location(2, 7, 0, 7, 7),
                params=OperatorParams(arity="unary"),
            ),
        ),
        # Constant with explicit value
        (
            elements.Constant(
                id=6,
                location=elements.Location(2, 2, 0, 5, 5),
                value="42",
                flType=FlType.I32,
            ),
            AddNode(
                parent=[2],
                new_location=elements.Location(2, 2, 0, 5, 5),
                params=ConstantParams(type=FlType.I32, value="42"),
            ),
        ),
        (
            elements.Constant(
                id=6,
                location=elements.Location(2, 2, 0, 5, 5),
                value="3.14",
                flType=FlType.F64,
            ),
            AddNode(
                parent=[2],
                new_location=elements.Location(2, 2, 0, 5, 5),
                params=ConstantParams(type=FlType.F64, value="3.14"),
            ),
        ),
        # Operator with explicit op
        (
            elements.BinaryOperator(
                id=6,
                location=elements.Location(15, 2, 1, 5, 5),
                op=elements.Operator.STAR,
            ),
            AddNode(
                parent=[2],
                new_location=elements.Location(15, 2, 1, 5, 5),
                params=OperatorParams(arity="binary", op="*"),
            ),
        ),
        (
            elements.UnaryOperator(
                id=6,
                location=elements.Location(2, 7, 0, 7, 7),
                op=elements.Operator.MINUS_MINUS,
            ),
            AddNode(
                parent=[2],
                new_location=elements.Location(2, 7, 0, 7, 7),
                params=OperatorParams(arity="unary", op="--"),
            ),
        ),
        # Operator with UNKNOWN op defaults to PLUS
        (
            elements.BinaryOperator(
                id=6,
                location=elements.Location(15, 2, 1, 5, 5),
                op=elements.Operator.PLUS,
            ),
            AddNode(
                parent=[2],
                new_location=elements.Location(15, 2, 1, 5, 5),
                params=OperatorParams(arity="binary", op=" "),
            ),
        ),
        (
            elements.Call(
                id=6,
                location=elements.Location(2, 7, 0, 7, 15),
                target="xyzzy",
                arguments=["a", "b"],
                returns=True,
            ),
            AddNode(
                parent=[2],
                new_location=elements.Location(2, 7, 0, 7, 7),
                params=CallParams(target="xyzzy"),
            ),
        ),
        # Calling an unknown function adds its name and empty args/return
        (
            elements.Call(
                id=6,
                location=elements.Location(2, 7, 0, 7, 5),
                target="unknown",
                arguments=[],
                returns=False,
            ),
            AddNode(
                parent=[2],
                new_location=elements.Location(2, 7, 0, 7, 7),
                params=CallParams(target="unknown"),
            ),
        ),
    ],
)
def test_add_node(
    expected_node: elements.Node,
    input: AddNode,
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].nodes.append(expected_node)

    input.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(input)
    actual = editor.get()

    assert expected == actual

    actual = input.undo(actual)
    assert original == actual


def test_add_node_missing_params_fails() -> None:
    with pytest.raises(ValidationError):
        AddNode(  # type: ignore[call-arg]
            parent=[2],
            new_location=elements.Location(2, 7, 0, 7, 7),
        )


def test_add_node_invalid_arity_fails() -> None:
    with pytest.raises(ValidationError):
        OperatorParams(arity="not_valid")  # type: ignore[arg-type]


def test_add_call_node_without_target_raises(
    basic_program: Program, editor: ModuleEditor
) -> None:
    """Adding a Call node with no target should raise BadEdit rather
    than silently inserting a placeholder '???' target."""
    uut = AddNode(
        parent=[2],
        new_location=elements.Location(2, 7, 0, 7, 7),
        params=CallParams(),
    )

    with pytest.raises(BadEdit):
        uut.do(basic_program)


def test_add_node_with_invalid_op_fails(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    uut = AddNode(
        parent=[2],
        new_location=elements.Location(2, 7, 0, 7, 7),
        params=OperatorParams(arity="binary", op="invalid"),
    )
    with pytest.raises(BadEdit):
        uut.resolve(intelligence, cast(Path, editor.get_path()))
        editor.edit(uut)


def test_add_call_node_can_add_builtin(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    expected_node = elements.Call(
        id=6,
        location=elements.Location(2, 7, 0, 7, 10),
        target="print",
        arguments=["object"],
        returns=False,
    )
    input = AddNode(
        parent=[2],
        new_location=elements.Location(2, 7, 0, 7, 7),
        params=CallParams(target="print"),
    )

    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].nodes.append(expected_node)

    input.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(input)
    actual = editor.get()

    assert expected == actual

    actual = input.undo(actual)
    assert original == actual

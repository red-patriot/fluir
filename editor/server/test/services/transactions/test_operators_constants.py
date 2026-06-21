import copy
from pathlib import Path
from typing import cast

import pytest

from editor.models import Program, elements
from editor.models.edit_errors import BadEdit
from editor.services.intelligence import IntelligenceReadInterface
from editor.services.module_editor import ModuleEditor
from editor.services.transaction import UpdateConstant, UpdateOperator


@pytest.mark.parametrize(
    "op",
    ["+", "-", "*", "/"],
)
def test_update_binary_operator(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
    op: str,
) -> None:
    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    assert isinstance(
        expected.declarations[1].nodes[0], elements.BinaryOperator
    )
    expected.declarations[1].nodes[0].op = elements.Operator(op)

    uut = UpdateOperator(target=[2, 1], value=op)
    uut.resolve(intelligence, cast(Path, editor.get_path()))

    editor.edit(uut)

    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


@pytest.mark.parametrize(
    "op",
    ["+", "-", "*", "/"],
)
def test_update_unary_operator(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
    op: str,
) -> None:
    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    assert isinstance(expected.declarations[2].nodes[0], elements.UnaryOperator)
    expected.declarations[2].nodes[0].op = elements.Operator(op)

    uut = UpdateOperator(target=[3, 2], value=op)
    uut.resolve(intelligence, cast(Path, editor.get_path()))

    editor.edit(uut)

    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_update_operator_raises_if_not_operator(
    editor: ModuleEditor, intelligence: IntelligenceReadInterface
) -> None:
    uut = UpdateOperator(target=[2, 2], value=elements.Operator.MINUS)
    with pytest.raises(
        BadEdit, match="Can only perform this update on an operator"
    ):
        uut.resolve(intelligence, cast(Path, editor.get_path()))
        editor.edit(uut)


def test_edit_constant(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    assert isinstance(expected.declarations[1].nodes[1], elements.Constant)
    expected.declarations[1].nodes[1].value = "-5.67"

    uut = UpdateConstant(target=[2, 2], value="-5.67")
    uut.resolve(intelligence, cast(Path, editor.get_path()))

    editor.edit(uut)

    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_edit_constant_raises_if_not_constant(
    editor: ModuleEditor, intelligence: IntelligenceReadInterface
) -> None:
    uut = UpdateConstant(target=[2, 1], value="-5.67")
    with pytest.raises(BadEdit):
        uut.resolve(intelligence, cast(Path, editor.get_path()))
        editor.edit(uut)

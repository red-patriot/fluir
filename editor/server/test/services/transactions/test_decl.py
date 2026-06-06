import copy
from pathlib import Path
from typing import cast

import pytest

from editor.models import FlType, Program, elements
from editor.models.edit_errors import BadEdit
from editor.services.intelligence import IntelligenceReadInterface
from editor.services.module_editor import ModuleEditor
from editor.services.transaction import (
    AddDecl,
    AddDeclInterface,
    DeclParameterParams,
    DeclReturnParams,
    EditTransaction,
    FunctionParams,
    RenameDeclaration,
    UpdateFuncParam,
    UpdateFuncReturn,
)


def test_add_decl(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations.append(
        elements.Function(
            id=5,
            name="new_function",
            location=elements.Location(50, 50, 0, 200, 200),
        )
    )

    uut = AddDecl(new_location=elements.Location(50, 50, 0, 200, 200))
    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_add_decl_with_custom_name(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations.append(
        elements.Function(
            id=5,
            name="my_func",
            location=elements.Location(50, 50, 0, 200, 200),
        )
    )

    uut = AddDecl(
        new_location=elements.Location(50, 50, 0, 200, 200),
        params=FunctionParams(name="my_func"),
    )
    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_add_func_parameter(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations[0].inputs = [
        elements.Parameter(name="new_param", id=1, flType=FlType.U8),
    ]

    uut = AddDeclInterface(
        parent=[1],
        flType=FlType.U8,
        params=DeclParameterParams(name="new_param"),
    )

    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert actual is not None
    assert actual == expected

    actual = uut.undo(actual)
    assert original == actual


def test_add_multiple_func_parameter(
    basic_program: Program, editor: ModuleEditor
) -> None:
    expected = copy.deepcopy(basic_program)
    expected.declarations[0].inputs = [
        elements.Parameter(name="new_param", id=1, flType=FlType.U8),
        elements.Parameter(name="other_param", id=2, flType=FlType.U32),
    ]

    uut1 = AddDeclInterface(
        parent=[1],
        flType=FlType.U8,
        params=DeclParameterParams(name="new_param"),
    )
    uut2 = AddDeclInterface(
        parent=[1],
        flType=FlType.U32,
        params=DeclParameterParams(name="other_param"),
    )

    editor.edit(uut1)
    editor.edit(uut2)
    actual = editor.get()

    assert actual is not None
    assert actual == expected


def test_add_decl_interface_raises_if_target_is_not_a_decl(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    """AddDeclInterface should reject a parent that points to a node
    inside a function rather than the function itself."""
    # [2, 1] points to the BinaryOperator inside function bar, not bar.
    uut = AddDeclInterface(
        parent=[2, 1],
        flType=FlType.U8,
        params=DeclParameterParams(name="x"),
    )

    with pytest.raises(BadEdit):
        uut.resolve(intelligence, cast(Path, editor.get_path()))
        editor.edit(uut)


def test_add_decl_interface_raises_if_decl_does_not_exist(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    """AddDeclInterface should surface find_element's lookup error when
    the parent ID does not resolve to any element."""
    uut = AddDeclInterface(
        parent=[9999],
        flType=FlType.U8,
        params=DeclParameterParams(name="x"),
    )

    with pytest.raises(elements.IdentifierError):
        uut.resolve(intelligence, cast(Path, editor.get_path()))
        editor.edit(uut)


def test_add_parameter_with_empty_name_raises(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    """A parameter must have a non-empty name."""
    uut = AddDeclInterface(
        parent=[1],
        flType=FlType.U8,
        params=DeclParameterParams(name=""),
    )

    with pytest.raises(BadEdit):
        uut.resolve(intelligence, cast(Path, editor.get_path()))
        editor.edit(uut)


def test_add_func_return(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations[0].outputs = [
        elements.Return(id=1, flType=FlType.U64)
    ]

    uut = AddDeclInterface(
        parent=[1],
        flType=FlType.U64,
        params=DeclReturnParams(),
    )

    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert actual is not None
    assert actual == expected

    actual = uut.undo(actual)
    assert original == actual


def test_rename_function(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].name = "baz"

    uut = RenameDeclaration(target=[2], name="baz")

    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_rename_declaration_raises_if_target_is_not_a_decl(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    uut = RenameDeclaration(target=[2, 1], name="baz")

    with pytest.raises(BadEdit, match="Only a declaration may be renamed"):
        uut.resolve(intelligence, cast(Path, editor.get_path()))
        editor.edit(uut)


def test_update_func_param_type(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    undone = copy.deepcopy(basic_program)
    done = copy.deepcopy(basic_program)
    done.declarations[3].inputs[0].flType = FlType.I64

    input = UpdateFuncParam(
        target=[4], index=0, cmd=UpdateFuncParam.UpdateType(flType=FlType.I64)
    )
    editor.edit(input)
    actual = editor.get()

    assert done == actual

    editor.undo()
    actual = editor.get()
    assert undone == actual


def test_update_func_param_name(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    undone = copy.deepcopy(basic_program)
    done = copy.deepcopy(basic_program)
    done.declarations[3].inputs[0].name = "new_param_name"

    input = UpdateFuncParam(
        target=[4],
        index=0,
        cmd=UpdateFuncParam.UpdateName(name="new_param_name"),
    )
    editor.edit(input)
    actual = editor.get()

    assert done == actual

    editor.undo()
    actual = editor.get()
    assert undone == actual


@pytest.mark.parametrize(
    "input",
    [
        UpdateFuncParam(
            target=[3, 2],
            index=0,
            cmd=UpdateFuncParam.UpdateType(flType=FlType.U64),
        ),
        UpdateFuncParam(
            target=[4],
            index=5,
            cmd=UpdateFuncParam.UpdateType(flType=FlType.U64),
        ),
    ],
)
def test_update_func_param_throws(
    basic_program: Program, editor: ModuleEditor, input: EditTransaction
) -> None:
    with pytest.raises(BadEdit):
        editor.edit(input)


def test_update_func_return(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    undone = copy.deepcopy(basic_program)
    done = copy.deepcopy(basic_program)
    done.declarations[3].outputs[0].flType = FlType.I64

    input = UpdateFuncReturn(target=[4], type=FlType.I64)

    editor.edit(input)
    actual = editor.get()

    assert done == actual

    editor.undo()
    actual = editor.get()
    assert undone == actual


@pytest.mark.parametrize(
    "input",
    [
        UpdateFuncReturn(target=[3, 2], type=FlType.U64),
        UpdateFuncReturn(target=[3], type=FlType.U64),
    ],
)
def test_update_func_return_throws(
    basic_program: Program, editor: ModuleEditor, input: EditTransaction
) -> None:
    with pytest.raises(BadEdit):
        editor.edit(input)

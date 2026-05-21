import copy
from pathlib import Path
from typing import cast

from editor.models import FlType, Program, elements
from editor.services.intelligence import IntelligenceReadInterface
from editor.services.module_editor import ModuleEditor
from editor.services.transaction import RemoveItem


def test_remove_node(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].nodes.pop(2)

    uut = RemoveItem(target=[2, 3])
    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_remove_node_with_conduits(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations[2].nodes.pop(1)
    expected.declarations[2].conduits.pop()

    uut = RemoveItem(target=[3, 3])
    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_remove_function(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations.pop()

    uut = RemoveItem(target=[4])
    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_remove_conduit(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].conduits.pop(0)

    uut = RemoveItem(target=[2, 5])
    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_remove_input(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    basic_program.declarations[1].inputs.append(
        elements.Parameter(name="x", id=10, flType=FlType.F64)
    )
    editor.open_module(copy.deepcopy(basic_program))

    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].inputs = []

    uut = RemoveItem(target=[2, 10])
    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_remove_input_with_conduits(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    basic_program.declarations[1].inputs.append(
        elements.Parameter(name="x", id=10, flType=FlType.F64)
    )
    basic_program.declarations[1].conduits.append(
        elements.Conduit(
            id=11,
            input=10,
            children=[elements.Conduit.Output(target=1, index=0)],
        )
    )
    editor.open_module(copy.deepcopy(basic_program))

    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].inputs = []
    expected.declarations[1].conduits = [
        c for c in expected.declarations[1].conduits if c.id != 11
    ]

    uut = RemoveItem(target=[2, 10])
    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_remove_output(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    basic_program.declarations[1].outputs.append(
        elements.Return(id=10, flType=FlType.F64)
    )
    editor.open_module(copy.deepcopy(basic_program))

    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].outputs = []

    uut = RemoveItem(target=[2, 10])
    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_remove_output_with_conduits(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    basic_program.declarations[1].outputs.append(
        elements.Return(id=10, flType=FlType.F64)
    )
    basic_program.declarations[1].conduits.append(
        elements.Conduit(
            id=11,
            input=1,
            children=[elements.Conduit.Output(target=10, index=0)],
        )
    )
    editor.open_module(copy.deepcopy(basic_program))

    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].outputs = []
    expected.declarations[1].conduits = [
        c for c in expected.declarations[1].conduits if c.id != 11
    ]

    uut = RemoveItem(target=[2, 10])
    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_remove_top_level_comment(
    program_with_comments: Program,
    comments_editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(program_with_comments)
    expected = copy.deepcopy(program_with_comments)
    expected.annotations.pop(0)

    uut = RemoveItem(target=[10])
    uut.resolve(intelligence, cast(Path, comments_editor.get_path()))
    comments_editor.edit(uut)
    actual = comments_editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_remove_in_body_comment(
    program_with_comments: Program,
    comments_editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(program_with_comments)
    expected = copy.deepcopy(program_with_comments)
    expected.declarations[1].annotations.pop(0)

    uut = RemoveItem(target=[2, 20])
    comments_editor.edit(uut)
    actual = comments_editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual

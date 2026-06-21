import copy
from pathlib import Path
from typing import cast

import pytest

from editor.models import Program, elements
from editor.models.edit_errors import BadEdit
from editor.services.intelligence import IntelligenceReadInterface
from editor.services.module_editor import ModuleEditor
from editor.services.transaction import AddComment, UpdateComment


def test_add_comment_top_level_to_empty_program(
    intelligence: IntelligenceReadInterface,
) -> None:
    program = Program()
    editor = ModuleEditor()
    editor.open_module(copy.deepcopy(program))

    uut = AddComment(
        parent=[],
        new_location=elements.Location(1, 2, 0, 30, 15),
        data="hello",
    )
    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()
    assert actual is not None
    assert len(actual.annotations) == 1
    assert actual.annotations[0].id == 1
    assert actual.annotations[0].data == "hello"
    assert actual.annotations[0].location == elements.Location(1, 2, 0, 30, 15)


def test_add_comment_top_level_uses_next_decl_id(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.annotations.append(
        elements.Comment(
            id=5,
            location=elements.Location(7, 8, 0, 25, 12),
            data="note",
        )
    )

    uut = AddComment(
        parent=[],
        new_location=elements.Location(7, 8, 0, 25, 12),
        data="note",
    )
    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_add_comment_in_body_uses_function_local_next_id(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].annotations.append(
        elements.Comment(
            id=6,
            location=elements.Location(4, 4, 0, 10, 6),
            data="inside",
        )
    )

    uut = AddComment(
        parent=[2],
        new_location=elements.Location(4, 4, 0, 10, 6),
        data="inside",
    )
    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_add_comment_undo_via_editor(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(basic_program)

    uut = AddComment(
        parent=[],
        new_location=elements.Location(1, 1, 0, 10, 5),
        data="x",
    )
    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    editor.undo()
    actual = editor.get()

    assert actual == original


def test_add_comment_redo_via_editor(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    expected = copy.deepcopy(basic_program)
    expected.annotations.append(
        elements.Comment(
            id=5,
            location=elements.Location(1, 1, 0, 10, 5),
            data="x",
        )
    )

    uut = AddComment(
        parent=[],
        new_location=elements.Location(1, 1, 0, 10, 5),
        data="x",
    )
    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    editor.undo()
    editor.redo()
    actual = editor.get()

    assert actual == expected


def test_add_comment_in_body_then_undo_redo(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].annotations.append(
        elements.Comment(
            id=6,
            location=elements.Location(2, 2, 0, 8, 4),
            data="hi",
        )
    )

    uut = AddComment(
        parent=[2],
        new_location=elements.Location(2, 2, 0, 8, 4),
        data="hi",
    )
    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    editor.undo()
    assert editor.get() == original
    editor.redo()
    assert editor.get() == expected


def test_update_comment_top_level(
    program_with_comments: Program,
    comments_editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(program_with_comments)
    expected = copy.deepcopy(program_with_comments)
    expected.annotations[0].data = "edited top-level"

    uut = UpdateComment(target=[10], data="edited top-level")
    uut.resolve(intelligence, cast(Path, comments_editor.get_path()))
    comments_editor.edit(uut)
    actual = comments_editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_update_comment_in_body(
    program_with_comments: Program,
    comments_editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(program_with_comments)
    expected = copy.deepcopy(program_with_comments)
    expected.declarations[1].annotations[0].data = "edited body"

    uut = UpdateComment(target=[2, 20], data="edited body")
    uut.resolve(intelligence, cast(Path, comments_editor.get_path()))
    comments_editor.edit(uut)
    actual = comments_editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_update_comment_redo_via_editor(
    program_with_comments: Program,
    comments_editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    expected = copy.deepcopy(program_with_comments)
    expected.annotations[0].data = "redo me"

    uut = UpdateComment(target=[10], data="redo me")
    uut.resolve(intelligence, cast(Path, comments_editor.get_path()))
    comments_editor.edit(uut)
    comments_editor.undo()
    comments_editor.redo()

    assert comments_editor.get() == expected


def test_update_comment_raises_if_not_comment(
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    uut = UpdateComment(target=[2, 1], data="nope")
    with pytest.raises(BadEdit):
        uut.resolve(intelligence, cast(Path, editor.get_path()))
        editor.edit(uut)

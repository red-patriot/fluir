import copy
from pathlib import Path
from typing import cast

import pytest

from editor.models import Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import find_element
from editor.services.intelligence import IntelligenceReadInterface
from editor.services.module_editor import ModuleEditor
from editor.services.transaction import MoveElement, ResizeElement


def test_move_function(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations[0].location = elements.Location(22, 57, 3, 100, 100)

    uut = MoveElement(target=[1], x=22, y=57)

    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_move_node(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(basic_program)
    expected = copy.deepcopy(basic_program)
    expected.declarations[1].nodes[2].location = elements.Location(
        6, 90, 1, 5, 5
    )

    uut = MoveElement(target=[2, 3], x=6, y=90)

    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


@pytest.mark.parametrize(
    "x, y",
    [(3, 101), (102, 13), (96, 12), (53, 97), (-3, 48), (0, -13), (-3, -13)],
)
# limit=elements.Location(210, 10, 2, 100, 100)
# element=elements.Location(2, 12, 1, 5, 5)
def test_move_node_raises_if_out_of_function(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
    x: int,
    y: int,
) -> None:
    uut = MoveElement(target=[2, 3], x=x, y=y)

    with pytest.raises(BadEdit):
        uut.resolve(intelligence, cast(Path, editor.get_path()))
        editor.edit(uut)


def test_move_top_level_comment(
    program_with_comments: Program,
    comments_editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(program_with_comments)
    expected = copy.deepcopy(program_with_comments)
    expected.annotations[0].location.x = 50
    expected.annotations[0].location.y = 60

    uut = MoveElement(target=[10], x=50, y=60)
    uut.resolve(intelligence, cast(Path, comments_editor.get_path()))
    comments_editor.edit(uut)
    actual = comments_editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


@pytest.mark.parametrize(
    "name, target, input, expected",
    [
        (
            "resize_node",
            [3, 3],
            {"width": 16, "height": 5},
            {"width": 16, "height": 5},
        ),
        (
            "resize_function_decl",
            [1],
            {"width": 160, "height": 150},
            {"width": 160, "height": 150},
        ),
        (
            "resize_function_decl_stays_larger_than_children",
            [2],
            {"width": 12, "height": 15},
            {"width": 20, "height": 17},
        ),
        (
            "resize_node_clamps_to_x_limits",
            [3, 3],
            {"width": 100, "height": 5},
            {"width": 98, "height": 5},
        ),
        (
            "resize_node_clamps_to_y_limits",
            [3, 3],
            {"width": 14, "height": 150},
            {"width": 14, "height": 98},
        ),
        (
            "resize_node_clamps_to_min_limits",
            [3, 3],
            {"width": 3, "height": -6},
            {"width": 4, "height": 4},
        ),
    ],
    ids=lambda x: x if isinstance(x, str) else "",
)
def test_resize_element(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
    target: QualifiedID,
    input: dict[str, int],
    expected: dict[str, int],
    name: str,
) -> None:
    original = copy.deepcopy(basic_program)
    expected_program = copy.deepcopy(basic_program)
    e = find_element(target, expected_program)
    e.location.width = expected["width"]
    e.location.height = expected["height"]

    uut = ResizeElement(
        target=target, width=input["width"], height=input["height"]
    )

    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert expected_program == actual

    actual = uut.undo(actual)
    assert original == actual


@pytest.mark.parametrize(
    "name, target, input, expected",
    [
        (
            "resize_node",
            [3, 3],
            {"width": 16, "height": 5, "x": 5, "y": 10},
            {"width": 16, "height": 5, "x": 5, "y": 10},
        ),
        (
            "resize_function_decl",
            [1],
            {"width": 160, "height": 150, "x": 20, "y": 30},
            {"width": 160, "height": 150, "x": 20, "y": 30},
        ),
        (
            "resize_function_decl_stays_larger_than_children",
            [2],
            {"width": 12, "height": 15, "x": 15, "y": 17},
            {"width": 20, "height": 17, "x": 15, "y": 17},
        ),
        (
            "resize_node_clamps_to_x_limits",
            [3, 3],
            {"width": 100, "height": 5, "x": 2, "y": 12},
            {"width": 98, "height": 5, "x": 2, "y": 12},
        ),
        (
            "resize_node_clamps_to_y_limits",
            [3, 3],
            {"width": 14, "height": 150, "x": 14, "y": 20},
            {"width": 14, "height": 98, "x": 14, "y": 20},
        ),
        (
            "resize_node_clamps_to_min_limits",
            [3, 3],
            {"width": 3, "height": -6, "x": 4, "y": 4},
            {"width": 4, "height": 4, "x": 4, "y": 4},
        ),
    ],
    ids=lambda x: x if isinstance(x, str) else "",
)
def test_resize_element_with_location(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
    target: QualifiedID,
    input: dict[str, int],
    expected: dict[str, int],
    name: str,
) -> None:
    original = copy.deepcopy(basic_program)
    expected_program = copy.deepcopy(basic_program)
    e = find_element(target, expected_program)
    e.location.width = expected["width"]
    e.location.height = expected["height"]
    e.location.x = expected["x"]
    e.location.y = expected["y"]

    uut = ResizeElement(
        target=target,
        width=input["width"],
        height=input["height"],
        x=input["x"],
        y=input["y"],
    )

    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert expected_program == actual

    actual = uut.undo(actual)
    assert original == actual


def test_resize_in_body_comment(
    program_with_comments: Program,
    comments_editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(program_with_comments)
    expected = copy.deepcopy(program_with_comments)
    expected.declarations[1].annotations[0].location.width = 25
    expected.declarations[1].annotations[0].location.height = 12

    uut = ResizeElement(target=[2, 20], width=25, height=12)
    uut.resolve(intelligence, cast(Path, comments_editor.get_path()))
    comments_editor.edit(uut)
    actual = comments_editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual

import copy
from pathlib import Path
from typing import cast

import pytest

from editor.models import Program, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import IdentifierError
from editor.services.intelligence import IntelligenceReadInterface
from editor.services.module_editor import ModuleEditor
from editor.services.transaction import (
    AddCallArg,
    DeleteCallArg,
    EditCallNode,
    RenameCallArg,
    ReorderCallArg,
)


def test_rename_call_arg(
    program_with_call: Program,
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    expected = copy.deepcopy(program_with_call)
    # call_host is declarations[4] (id=100); Call node is nodes[0] (id=1)
    call_node = expected.declarations[4].nodes[0]
    assert isinstance(call_node, elements.Call)
    call_node.arguments[0] = "renamed_x"

    uut = EditCallNode(
        target=[100, 1],
        command=RenameCallArg(index=0, name="renamed_x"),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    call_editor.edit(uut)

    actual = call_editor.get()
    assert expected == actual


def test_rename_call_arg_undo(
    program_with_call: Program,
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(program_with_call)

    uut = EditCallNode(
        target=[100, 1],
        command=RenameCallArg(index=0, name="renamed_x"),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    call_editor.edit(uut)

    actual = call_editor.get()
    assert actual is not None
    actual = uut.undo(actual)
    assert original == actual


def test_rename_call_arg_wrong_id(
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    uut = EditCallNode(
        target=[99, 99],
        command=RenameCallArg(index=0, name="renamed_x"),
    )
    with pytest.raises(IdentifierError):
        uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
        call_editor.edit(uut)


def test_rename_call_arg_wrong_index(
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    uut = EditCallNode(
        target=[100, 1],
        command=RenameCallArg(index=99, name="renamed_x"),
    )
    with pytest.raises(IndexError):
        uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
        call_editor.edit(uut)


def test_add_call_arg(
    program_with_call: Program,
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    expected = copy.deepcopy(program_with_call)
    call_node = expected.declarations[4].nodes[0]
    assert isinstance(call_node, elements.Call)
    call_node.arguments.append("z")

    uut = EditCallNode(
        target=[100, 1],
        command=AddCallArg(name="z"),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    call_editor.edit(uut)

    assert expected == call_editor.get()


def test_add_call_arg_undo(
    program_with_call: Program,
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(program_with_call)

    uut = EditCallNode(
        target=[100, 1],
        command=AddCallArg(name="z"),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    call_editor.edit(uut)

    actual = call_editor.get()
    assert actual is not None
    actual = uut.undo(actual)
    assert original == actual


def test_delete_call_arg(
    program_with_call: Program,
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    expected = copy.deepcopy(program_with_call)
    call_node = expected.declarations[4].nodes[0]
    assert isinstance(call_node, elements.Call)
    call_node.arguments.pop(0)  # removes "x", leaves ["y"]

    uut = EditCallNode(
        target=[100, 1],
        command=DeleteCallArg(index=0),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    call_editor.edit(uut)

    assert expected == call_editor.get()


def test_delete_call_arg_undo(
    program_with_call: Program,
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(program_with_call)

    uut = EditCallNode(
        target=[100, 1],
        command=DeleteCallArg(index=0),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    call_editor.edit(uut)

    actual = call_editor.get()
    assert actual is not None
    actual = uut.undo(actual)
    assert original == actual


def test_delete_call_arg_out_of_bounds(
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    uut = EditCallNode(
        target=[100, 1],
        command=DeleteCallArg(index=99),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    with pytest.raises(BadEdit):
        call_editor.edit(uut)


def test_delete_call_arg_negative_index(
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    uut = EditCallNode(
        target=[100, 1],
        command=DeleteCallArg(index=-1),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    with pytest.raises(BadEdit):
        call_editor.edit(uut)


def test_reorder_call_arg(
    program_with_call: Program,
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    expected = copy.deepcopy(program_with_call)
    call_node = expected.declarations[4].nodes[0]
    assert isinstance(call_node, elements.Call)
    call_node.arguments = ["y", "x"]

    uut = EditCallNode(
        target=[100, 1],
        command=ReorderCallArg(current=0, destination=1),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    call_editor.edit(uut)

    assert expected == call_editor.get()


def test_reorder_call_arg_undo(
    program_with_call: Program,
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(program_with_call)

    uut = EditCallNode(
        target=[100, 1],
        command=ReorderCallArg(current=0, destination=1),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    call_editor.edit(uut)

    actual = call_editor.get()
    assert actual is not None
    actual = uut.undo(actual)
    assert original == actual


def test_reorder_call_arg_current_out_of_bounds(
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    uut = EditCallNode(
        target=[100, 1],
        command=ReorderCallArg(current=5, destination=0),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    with pytest.raises(BadEdit):
        call_editor.edit(uut)


def test_reorder_call_arg_destination_out_of_bounds(
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    uut = EditCallNode(
        target=[100, 1],
        command=ReorderCallArg(current=0, destination=5),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    with pytest.raises(BadEdit):
        call_editor.edit(uut)


def test_reorder_call_arg_negative_current(
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    uut = EditCallNode(
        target=[100, 1],
        command=ReorderCallArg(current=-1, destination=0),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    with pytest.raises(BadEdit):
        call_editor.edit(uut)


def test_reorder_call_arg_negative_destination(
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    uut = EditCallNode(
        target=[100, 1],
        command=ReorderCallArg(current=0, destination=-1),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    with pytest.raises(BadEdit):
        call_editor.edit(uut)

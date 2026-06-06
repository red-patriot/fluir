import copy
from pathlib import Path
from typing import cast

import pytest

from editor.models import Program, elements
from editor.models.elements import IdentifierError
from editor.services.intelligence import IntelligenceReadInterface
from editor.services.module_editor import ModuleEditor
from editor.services.transaction import EditCallNode, RenameCallArg


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

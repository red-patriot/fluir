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
    AddCallReturn,
    DeleteCallArg,
    DeleteCallReturn,
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
    call_node.location.height += 5

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
    expected.declarations[4].conduits.clear()  # remove the conduit going to "x"
    call_node.location.height -= 5

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
        command=DeleteCallArg(index=1),
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


def test_delete_call_arg_shifts_conduit_indices(
    program_with_call: Program,
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    # Add a conduit targeting arg index 1 of the call node (id=1)
    program_with_call.declarations[4].conduits.append(
        elements.Conduit(
            id=2,
            input=2,
            children=[elements.Conduit.Output(target=1, index=1)],
        )
    )
    call_editor.open_module(copy.deepcopy(program_with_call))

    uut = EditCallNode(
        target=[100, 1],
        command=DeleteCallArg(index=0),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    call_editor.edit(uut)

    result = call_editor.get()
    assert result is not None
    conduits = result.declarations[4].conduits
    # Conduit to index 0 was removed; conduit formerly at index 1 shifted to 0
    assert len(conduits) == 1
    out = conduits[0].children[0]
    assert isinstance(out, elements.Conduit.Output)
    assert out.index == 0


def test_delete_call_arg_shifts_conduit_indices_undo(
    program_with_call: Program,
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    # Use only a conduit at index 1 (no conduit at index 0) to avoid ordering issues
    program_with_call.declarations[4].conduits = [
        elements.Conduit(
            id=2,
            input=2,
            children=[elements.Conduit.Output(target=1, index=1)],
        )
    ]
    original = copy.deepcopy(program_with_call)
    call_editor.open_module(copy.deepcopy(program_with_call))

    uut = EditCallNode(
        target=[100, 1],
        command=DeleteCallArg(index=0),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    call_editor.edit(uut)

    result = call_editor.get()
    assert result is not None
    result = uut.undo(result)
    assert original == result


def test_delete_call_arg_undo_restores_conduits(
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

    result = call_editor.get()
    assert result is not None
    result = uut.undo(result)
    assert original == result


def test_reorder_call_arg(
    program_with_call: Program,
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    expected = copy.deepcopy(program_with_call)
    call_node = expected.declarations[6].nodes[0]
    conduits = expected.declarations[6].conduits
    assert isinstance(call_node, elements.Call)
    call_node.arguments = ["y", "x"]
    cast(elements.Conduit.Output, conduits[0].children[0]).index = 1
    cast(elements.Conduit.Output, conduits[1].children[0]).index = 0

    uut = EditCallNode(
        target=[102, 1],
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
        target=[102, 1],
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


def test_add_call_return(
    program_with_call: Program,
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    expected = copy.deepcopy(program_with_call)
    call_node = expected.declarations[4].nodes[0]
    assert isinstance(call_node, elements.Call)
    call_node.returns = True

    uut = EditCallNode(
        target=[100, 1],
        command=AddCallReturn(),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    call_editor.edit(uut)

    assert expected == call_editor.get()


def test_add_call_return_undo(
    program_with_call: Program,
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(program_with_call)

    uut = EditCallNode(
        target=[100, 1],
        command=AddCallReturn(),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    call_editor.edit(uut)

    actual = call_editor.get()
    assert actual is not None
    actual = uut.undo(actual)
    assert original == actual


def test_add_call_return_already_has_return(
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    uut = EditCallNode(
        target=[101, 1],
        command=AddCallReturn(),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    with pytest.raises(BadEdit):
        call_editor.edit(uut)


def test_delete_call_return(
    program_with_call: Program,
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    expected = copy.deepcopy(program_with_call)
    call_node = expected.declarations[5].nodes[0]
    assert isinstance(call_node, elements.Call)
    call_node.returns = False
    expected.declarations[5].conduits.clear()

    uut = EditCallNode(
        target=[101, 1],
        command=DeleteCallReturn(),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    call_editor.edit(uut)

    assert expected == call_editor.get()


def test_delete_call_return_undo(
    program_with_call: Program,
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(program_with_call)

    uut = EditCallNode(
        target=[101, 1],
        command=DeleteCallReturn(),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    call_editor.edit(uut)

    actual = call_editor.get()
    assert actual is not None
    actual = uut.undo(actual)
    assert original == actual


def test_delete_call_return_no_return(
    call_editor: ModuleEditor,
    call_intelligence: IntelligenceReadInterface,
) -> None:
    uut = EditCallNode(
        target=[100, 1],
        command=DeleteCallReturn(),
    )
    uut.resolve(call_intelligence, cast(Path, call_editor.get_path()))
    with pytest.raises(BadEdit):
        call_editor.edit(uut)

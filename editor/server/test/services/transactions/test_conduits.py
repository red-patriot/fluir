import copy
from pathlib import Path
from typing import cast

from editor.models import Program, elements
from editor.services.intelligence import IntelligenceReadInterface
from editor.services.module_editor import ModuleEditor
from editor.services.transaction import AddConduit


def test_add_conduit_no_conflicts(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
    original = copy.deepcopy(basic_program)
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
    uut.resolve(intelligence, cast(Path, editor.get_path()))

    editor.edit(uut)

    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual


def test_add_conduit_removes_duplicate_targets(
    basic_program: Program,
    editor: ModuleEditor,
    intelligence: IntelligenceReadInterface,
) -> None:
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

    original = copy.deepcopy(program)
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

    uut = AddConduit(target="input-2:1-0", source="output-2:2-0")
    uut.resolve(intelligence, cast(Path, editor.get_path()))
    editor.edit(uut)
    actual = editor.get()

    assert expected == actual

    actual = uut.undo(actual)
    assert original == actual

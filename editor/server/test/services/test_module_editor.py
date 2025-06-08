import copy
import os
from pathlib import Path
from typing import Iterator
from unittest.mock import create_autospec

import pytest

from editor.models import elements
from editor.models.edit_errors import BadEdit, EditorError
from editor.models.elements import (
    FlType,
    Function,
    Location,
    Program,
    find_element,
)
from editor.services import transaction
from editor.services.module_editor import ModuleEditor
from editor.services.transaction import EditTransaction, TransactionBase


@pytest.fixture
def test_file1(tmp_path: Path) -> Iterator[Path]:
    filename = tmp_path / "test.fl"
    with open(filename, "w") as file:
        file.write("""<?xml version="1.0" encoding="UTF-8"?>
        <fluir>
            <function
                name="bar"
                id="174"
                x="10" y="10" z="3" w="100" h="100">
                <body>
                </body>
            </function>
        </fluir>
        """)
    yield filename
    os.remove(filename)


@pytest.fixture
def test_file2(tmp_path: Path) -> Iterator[Path]:
    filename = tmp_path / "test2.fl"
    with open(filename, "w") as file:
        file.write("""<?xml version="1.0" encoding="UTF-8"?>
        <fluir>
            <function
                name="foo"
                id="2"
                x="10" y="10" z="3" w="12" h="100">
                <body>
                </body>
            </function>
        </fluir>
        """)
    yield filename
    os.remove(filename)


@pytest.fixture
def basic_program() -> Program:
    return Program(
        [
            elements.Function(
                name="qux",
                location=elements.Location(210, 10, 2, 100, 110),
                id=3,
                nodes=[
                    elements.UnaryOperator(
                        id=2,
                        location=elements.Location(15, 2, 1, 5, 5),
                        op=elements.Operator.MINUS,
                    ),
                    elements.Constant(
                        id=3,
                        location=elements.Location(2, 12, 1, 5, 5),
                        value="2.0",
                        flType=FlType.FLOATING_POINT,
                    ),
                ],
                conduits=[
                    elements.Conduit(
                        id=5,
                        input=3,
                        children=[elements.Conduit.Output(target=2, index=0)],
                    )
                ],
            ),
        ]
    )


def test_module_editor_can_open_files(test_file1: Path) -> None:
    expected = Program(
        [
            Function(
                name="bar",
                location=Location(10, 10, 3, 100, 100),
                id=174,
            )
        ]
    )
    expected_name = "test.fl"

    uut = ModuleEditor()

    uut.open_file(test_file1)

    assert test_file1 == uut.get_path()
    assert expected == uut.get()
    assert expected_name == uut.get_name()


def test_module_editor_can_close_files(test_file1: Path) -> None:
    uut = ModuleEditor()

    uut.open_file(test_file1)

    assert uut.get_path is not None

    uut.close()

    assert uut.get() is None
    assert uut.get_path() is None
    assert uut.get_name() == ""


def test_module_editor_can_close_automatically_on_open_files(
    test_file1: Path, test_file2: Path
) -> None:
    expected_name = "test2.fl"
    expected = Program(
        [
            Function(
                name="foo",
                location=Location(10, 10, 3, 12, 100),
                id=2,
            )
        ]
    )

    uut = ModuleEditor()

    uut.open_file(test_file1)
    uut.open_file(test_file2)

    assert test_file2 == uut.get_path()
    assert expected == uut.get()
    assert expected_name == uut.get_name()


def test_module_editor_accepts_edit_transactions() -> None:
    data = Program(
        [
            Function(
                name="function1Here",
                location=Location(10, 10, 3, 12, 100),
                id=2,
            )
        ]
    )
    fake_command = create_autospec(TransactionBase, instance=True)
    fake_command.do.return_value = data

    uut = ModuleEditor()
    uut.open_module(data)

    uut.edit(fake_command)

    fake_command.do.assert_called()


def test_module_editor_edit_requires_an_open_program() -> None:
    fake_command = create_autospec(EditTransaction, instance=True)

    uut = ModuleEditor()

    with pytest.raises(BadEdit):
        uut.edit(fake_command)


def test_module_editor_can_save_files(tmp_path: Path) -> None:
    expected = """<?xml version='1.0' encoding='UTF-8'?>
<fluir>
  <function name="bar" id="174" x="10" y="10" z="3" w="100" h="100">
    <body/>
  </function>
</fluir>
"""
    data = Program(
        [
            Function(
                name="bar",
                location=Location(10, 10, 3, 100, 100),
                id=174,
            )
        ]
    )
    expected_path = tmp_path / "module.fl"

    uut = ModuleEditor()
    uut.open_module(data, expected_path)
    uut.save_file()

    with open(expected_path, "r") as file:
        actual = file.read()

    assert expected == actual


def test_module_editor_raises_if_no_module() -> None:
    uut = ModuleEditor()

    with pytest.raises(EditorError):
        uut.save_file()


def test_module_editor_raises_if_no_file_path() -> None:
    data = Program(
        [
            Function(
                name="main",
                location=Location(10, 10, 3, 100, 100),
                id=1,
            )
        ]
    )
    uut = ModuleEditor()
    uut.open_module(data, None)

    with pytest.raises(EditorError):
        uut.save_file()


def test_module_editor_accepts_explicit_file_path(tmp_path: Path) -> None:
    expected = """<?xml version='1.0' encoding='UTF-8'?>
<fluir>
  <function name="main" id="1" x="10" y="10" z="3" w="100" h="100">
    <body/>
  </function>
</fluir>
"""
    data = Program(
        [
            Function(
                name="main",
                location=Location(10, 10, 3, 100, 100),
                id=1,
            )
        ]
    )
    expected_path = tmp_path / "explicitly_specified.fl"

    uut = ModuleEditor()
    uut.open_module(data, None)

    uut.save_file(expected_path)

    with open(expected_path, "r") as file:
        actual = file.read()

    assert expected == actual
    assert expected_path == uut.get_path(), "Should switch to new path"


def test_module_editor_can_open_new_file() -> None:
    # For now, create an empty main when making a new module
    expected = Program(
        [
            Function(
                name="main",
                location=Location(0, 0, 0, 100, 100),
                id=1,
            )
        ]
    )

    uut = ModuleEditor()
    uut.new_module()

    assert uut.get_path() is None
    assert uut.get_name() == ""
    assert expected == uut.get()


@pytest.mark.parametrize(
    "action",
    [
        transaction.MoveElement(target=[3, 3], x=6, y=90),
        transaction.RemoveItem(target=[3, 5]),
    ],
    ids=lambda x: x.__class__,
)
def test_undo(basic_program: Program, action: EditTransaction) -> None:
    expected = copy.deepcopy(basic_program)

    uut = ModuleEditor()
    uut.open_module(basic_program)

    uut.edit(action)
    edited = uut.get()
    assert expected != edited
    assert uut.can_undo()

    uut.undo()
    undone = uut.get()
    assert expected == undone
    assert not uut.can_undo()
    assert uut.can_redo()


@pytest.mark.parametrize(
    "action",
    [
        transaction.MoveElement(target=[3, 3], x=6, y=90),
        transaction.RemoveItem(target=[3, 5]),
    ],
    ids=lambda x: x.__class__,
)
def test_redo(basic_program: Program, action: EditTransaction) -> None:
    uut = ModuleEditor()
    uut.open_module(basic_program)

    uut.edit(action)
    expected = copy.deepcopy(uut.get())
    uut.undo()

    assert uut.can_redo()
    uut.redo()
    redone = uut.get()

    assert expected == redone
    assert not uut.can_redo()
    assert uut.can_undo()


def test_undo_max_size(basic_program: Program) -> None:
    uut = ModuleEditor(stack_max=10)
    uut.open_module(basic_program)
    action = transaction.MoveElement(target=[3, 3], x=6, y=90)

    for _ in range(20):
        uut.edit(action)

    for _ in range(11):
        uut.undo()

    assert not uut.can_undo()


def test_redo_max_size(basic_program: Program) -> None:
    uut = ModuleEditor(stack_max=10)
    uut.open_module(basic_program)
    action = transaction.MoveElement(target=[3, 3], x=6, y=90)

    for _ in range(20):
        uut.edit(action)
        uut.undo()

    for _ in range(11):
        uut.redo()

    assert not uut.can_redo()

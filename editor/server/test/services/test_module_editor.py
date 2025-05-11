import os
from pathlib import Path
from typing import Iterator
from unittest.mock import create_autospec

import pytest

from editor.models import EditTransaction
from editor.models.elements import Function, Location, Program
from editor.services.module_editor import BadEdit, ModuleEditor


@pytest.fixture
def test_file1(tmp_path: Path) -> Iterator[Path]:
    filename = tmp_path / "test.fl"
    with open(filename, "w") as file:
        file.write("""<?xml version="1.0" encoding="UTF-8"?>
        <fluir xmlns:fl="FLUIR::LANGUAGE::SOURCE">
            <fl:function
                name="bar"
                id="174"
                x="10" y="10" z="3" w="100" h="100">
                <body>
                </body>
            </fl:function>
        </fluir>
        """)
    yield filename
    os.remove(filename)


@pytest.fixture
def test_file2(tmp_path: Path) -> Iterator[Path]:
    filename = tmp_path / "test2.fl"
    with open(filename, "w") as file:
        file.write("""<?xml version="1.0" encoding="UTF-8"?>
        <fluir xmlns:fl="FLUIR::LANGUAGE::SOURCE">
            <fl:function
                name="foo"
                id="2"
                x="10" y="10" z="3" w="12" h="100">
                <body>
                </body>
            </fl:function>
        </fluir>
        """)
    yield filename
    os.remove(filename)


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
    fake_command = create_autospec(EditTransaction, instance=True)
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

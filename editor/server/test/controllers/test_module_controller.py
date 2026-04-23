from pathlib import Path
from unittest.mock import MagicMock

import pytest
from fastapi import FastAPI
from fastapi.testclient import TestClient

from editor.controllers.module_controller import ModuleController
from editor.models import Function, Program
from editor.models.module_requests import OpenRequest, SaveRequest
from editor.models.version import FLUIR_CURRENT_VERSION as VERS
from editor.services.intelligence import IntelligenceService
from editor.services.module_editor import ModuleEditor
from editor.services.transaction import MoveElement, UpdateConstant


@pytest.fixture
def mock_editor() -> MagicMock:
    fake = MagicMock(spec=ModuleEditor)
    fake.get.return_value = Program(declarations=[])
    fake.can_undo.return_value = True
    fake.can_redo.return_value = True

    return fake


@pytest.fixture
def mock_intelligence() -> MagicMock:
    return MagicMock(spec=IntelligenceService)


def test_forwards_open_request(
    mock_editor: MagicMock, mock_intelligence: MagicMock
) -> None:
    expectedPath = "/fake/path/to/module.fl"

    uut = ModuleController(mock_editor, mock_intelligence)

    uut.open(OpenRequest(path=expectedPath))

    mock_editor.open_file.assert_called_with(Path(expectedPath))


def test_forwards_open_request_on_post(
    mock_editor: MagicMock, mock_intelligence: MagicMock
) -> None:
    expectedPath = "/fake/path/to/module.fl"
    expectedData = f'{{"declarations":[],"header":{{"version":{{"MAJOR":{VERS.MAJOR},"MINOR":{VERS.MINOR},"PATCH":{VERS.PATCH}}}}}}}'

    app = FastAPI()

    uut = ModuleController(mock_editor, mock_intelligence)
    uut.register(app)
    testApp = TestClient(app)

    response = testApp.post("/api/module/open/", json={"path": expectedPath})

    assert response.status_code == 200
    assert expectedData in response.text
    mock_editor.open_file.assert_called_with(Path(expectedPath))


def test_forwards_new_request_on_post(
    mock_editor: MagicMock, mock_intelligence: MagicMock
) -> None:
    app = FastAPI()

    uut = ModuleController(mock_editor, mock_intelligence)
    uut.register(app)
    testApp = TestClient(app)

    response = testApp.post("/api/module/new/")

    assert response.status_code == 200
    mock_editor.new_module.assert_called()


def test_forwards_close_request(
    mock_editor: MagicMock, mock_intelligence: MagicMock
) -> None:
    uut = ModuleController(mock_editor, mock_intelligence)

    uut.close()

    mock_editor.close.assert_called()


def test_forwards_close_request_on_post(
    mock_editor: MagicMock, mock_intelligence: MagicMock
) -> None:
    app = FastAPI()

    uut = ModuleController(mock_editor, mock_intelligence)
    uut.register(app)
    testApp = TestClient(app)

    response = testApp.post("/api/module/close/")

    assert response.status_code == 200
    mock_editor.close.assert_called()


def test_forwards_move_request_on_post(
    mock_editor: MagicMock, mock_intelligence: MagicMock
) -> None:
    expected = MoveElement(target=[1, 2], x=13, y=19)

    app = FastAPI()

    uut = ModuleController(mock_editor, mock_intelligence)
    uut.register(app)
    testApp = TestClient(app)

    response = testApp.post(
        "/api/module/edit/",
        json={"discriminator": "move", "target": [1, 2], "x": 13, "y": 19},
    )

    assert response.status_code == 200
    mock_editor.edit.assert_called_with(expected)


def test_forwards_edit_constant_request_on_post(
    mock_editor: MagicMock, mock_intelligence: MagicMock
) -> None:
    expected = UpdateConstant(target=[1, 2, 4], value="-5.432")

    app = FastAPI()

    uut = ModuleController(mock_editor, mock_intelligence)
    uut.register(app)
    testApp = TestClient(app)

    response = testApp.post(
        "/api/module/edit/",
        json={
            "discriminator": "update_constant",
            "target": [1, 2, 4],
            "value": "-5.432",
        },
    )

    assert response.status_code == 200
    mock_editor.edit.assert_called_with(expected)


def test_forwards_save_request_on_post(
    mock_editor: MagicMock, mock_intelligence: MagicMock
) -> None:
    app = FastAPI()

    uut = ModuleController(mock_editor, mock_intelligence)
    uut.register(app)
    testApp = TestClient(app)

    response = testApp.post(
        "/api/module/save/",
        json={"path": ""},
    )

    assert response.status_code == 200
    mock_editor.save_file.assert_called()


def test_forwards_save_request_parameters_on_post(
    mock_editor: MagicMock, mock_intelligence: MagicMock
) -> None:
    app = FastAPI()

    uut = ModuleController(mock_editor, mock_intelligence)
    uut.register(app)
    testApp = TestClient(app)

    response = testApp.post(
        "/api/module/save/",
        json={"path": "/path/to/save/module.fl"},
    )

    assert response.status_code == 200
    mock_editor.save_file.assert_called_with(Path("/path/to/save/module.fl"))


def test_forwards_undo_constant_request_on_post(
    mock_editor: MagicMock, mock_intelligence: MagicMock
) -> None:
    app = FastAPI()

    uut = ModuleController(mock_editor, mock_intelligence)
    uut.register(app)
    testApp = TestClient(app)

    response = testApp.post(
        "/api/module/undo/",
    )

    assert response.status_code == 200
    mock_editor.undo.assert_called()
    mock_editor.can_undo.assert_called()
    mock_editor.can_redo.assert_called()


def test_prevents_undo_if_not_possible(
    mock_editor: MagicMock, mock_intelligence: MagicMock
) -> None:
    app = FastAPI()

    mock_editor.can_undo.return_value = False
    uut = ModuleController(mock_editor, mock_intelligence)
    uut.register(app)
    testApp = TestClient(app)

    response = testApp.post(
        "/api/module/undo/",
    )

    assert response.status_code == 412


def test_forwards_redo_constant_request_on_post(
    mock_editor: MagicMock, mock_intelligence: MagicMock
) -> None:
    app = FastAPI()

    uut = ModuleController(mock_editor, mock_intelligence)
    uut.register(app)
    testApp = TestClient(app)

    response = testApp.post(
        "/api/module/redo/",
    )

    assert response.status_code == 200
    mock_editor.redo.assert_called()
    mock_editor.can_undo.assert_called()
    mock_editor.can_redo.assert_called()


def test_prevents_redo_if_not_possible(
    mock_editor: MagicMock, mock_intelligence: MagicMock
) -> None:
    app = FastAPI()

    mock_editor.can_redo.return_value = False
    uut = ModuleController(mock_editor, mock_intelligence)
    uut.register(app)
    testApp = TestClient(app)

    response = testApp.post(
        "/api/module/redo/",
    )

    assert response.status_code == 412


def test_open_registers_module_with_intelligence(
    mock_editor: MagicMock,
) -> None:
    path = Path("/fake/path/to/module.fl")
    program = Program(declarations=[Function(name="foo", id=1)])
    mock_editor.get.return_value = program
    mock_editor.get_path.return_value = path
    intelligence = IntelligenceService()

    uut = ModuleController(mock_editor, intelligence)
    uut.open(OpenRequest(path=str(path)))

    completions = intelligence.get_completions([1], path)
    assert any(c.short_name == "foo" for c in completions)


def test_close_removes_module_from_intelligence(
    mock_editor: MagicMock,
) -> None:
    path = Path("/fake/path/to/module.fl")
    program = Program(declarations=[Function(name="foo", id=1)])
    mock_editor.get.return_value = program
    mock_editor.get_path.return_value = path
    intelligence = IntelligenceService()
    intelligence.add_module(program, path)

    uut = ModuleController(mock_editor, intelligence)
    uut.close()

    assert intelligence.get_completions([1], path) == []

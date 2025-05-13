from pathlib import Path
from unittest.mock import MagicMock

import pytest
from fastapi import FastAPI
from fastapi.testclient import TestClient

from editor.controllers.module_controller import ModuleController
from editor.models import Program
from editor.models.module_requests import OpenRequest
from editor.services.module_editor import ModuleEditor
from editor.services.transaction import MoveElement


@pytest.fixture
def mock_editor() -> MagicMock:
    fake = MagicMock(spec=ModuleEditor)
    fake.get.return_value = Program()

    return fake


def test_forwards_open_request(mock_editor: MagicMock) -> None:
    expectedPath = "/fake/path/to/module.fl"

    uut = ModuleController(mock_editor)

    uut.open(OpenRequest(path=expectedPath))

    mock_editor.open_file.assert_called_with(Path(expectedPath))


def test_forwards_open_request_on_post(mock_editor: MagicMock) -> None:
    expectedPath = "/fake/path/to/module.fl"
    expectedData = '{"declarations":[]}'

    app = FastAPI()

    uut = ModuleController(mock_editor)
    uut.register(app)
    testApp = TestClient(app)

    response = testApp.post("/api/module/open/", json={"path": expectedPath})

    assert response.status_code == 200
    assert expectedData == response.text
    mock_editor.open_file.assert_called_with(Path(expectedPath))


def test_forwards_close_request(mock_editor: MagicMock) -> None:
    uut = ModuleController(mock_editor)

    uut.close()

    mock_editor.close.assert_called()


def test_forwards_close_request_on_post(mock_editor: MagicMock) -> None:
    app = FastAPI()

    uut = ModuleController(mock_editor)
    uut.register(app)
    testApp = TestClient(app)

    response = testApp.post("/api/module/close/")

    assert response.status_code == 200
    mock_editor.close.assert_called()


def test_forwards_edit_request_on_post(mock_editor: MagicMock) -> None:
    expected = MoveElement(target=[1, 2], x=13, y=19)

    app = FastAPI()

    uut = ModuleController(mock_editor)
    uut.register(app)
    testApp = TestClient(app)

    response = testApp.post(
        "/api/module/edit/",
        json={"_t": "move", "target": [1, 2], "x": 13, "y": 19},
    )

    assert response.status_code == 200
    mock_editor.edit.assert_called_with(expected)


def test_forwards_save_request_on_post(mock_editor: MagicMock) -> None:
    app = FastAPI()

    uut = ModuleController(mock_editor)
    uut.register(app)
    testApp = TestClient(app)

    response = testApp.post(
        "/api/module/save/",
        json={"path": ""},
    )

    assert response.status_code == 200
    mock_editor.save_file.assert_called()


def test_forwards_save_request_parameters_on_post(
    mock_editor: MagicMock,
) -> None:
    app = FastAPI()

    uut = ModuleController(mock_editor)
    uut.register(app)
    testApp = TestClient(app)

    response = testApp.post(
        "/api/module/save/",
        json={"path": "/path/to/save/module.fl"},
    )

    assert response.status_code == 200
    mock_editor.save_file.assert_called_with(Path("/path/to/save/module.fl"))

from typing import Final

import uvicorn
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from editor.controllers import ModuleController
from editor.repository.fluir_file import XMLFileManager
from editor.services import ModuleEditor


def _setup_server() -> FastAPI:
    # TODO: Get these from a parameter
    origins: Final = ["http://localhost:5173", "http://127.0.0.1:5173"]
    app = FastAPI()
    app.add_middleware(
        CORSMiddleware,
        allow_origins=origins,
        allow_credentials=True,
        allow_methods=["*"],
        allow_headers=["*"],
    )
    return app


def _run_server(app: FastAPI) -> None:
    # TODO: Get the port from settings here
    uvicorn.run(app, port=8001)


def main() -> None:
    app = _setup_server()

    module_controller = ModuleController(ModuleEditor(XMLFileManager()))
    module_controller.register(app)

    _run_server(app)


if __name__ == "__main__":
    main()

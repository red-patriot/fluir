from typing import Any

from lxml.objectify import ObjectifiedElement, fromstring

from editor.models.elements import (
    Declaration,
    Function,
    IDType,
    Location,
    Program,
)


class FileManager:
    """"""

    def parseStr(self, source: bytes) -> Program:
        root = fromstring(source)
        return self._program(root)

    def _program(self, root: ObjectifiedElement) -> Program:
        declarations: dict[IDType, Declaration] = {}
        for element in root.iterchildren():
            id, decl = self._declaration(element)
            if id in declarations.keys():
                raise ValueError(f"{id} is repeated in the document")
            declarations[id] = decl

        return Program(declarations)

    def _declaration(self, element: Any) -> tuple[IDType, Declaration]:
        return int(element.get("id")), Function(
            name=str(element.get("name")),
            id=int(element.get("id")),
            location=self._location(element),
            body={},
        )

    def _location(self, element: Any) -> Location:
        return Location(
            x=int(element.get("x")),
            y=int(element.get("y")),
            z=int(element.get("z")),
            width=int(element.get("w")),
            height=int(element.get("h")),
        )

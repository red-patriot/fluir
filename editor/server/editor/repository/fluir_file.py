from typing import Any

from lxml.objectify import ObjectifiedElement, fromstring

from editor.models.elements import (
    BinaryOperator,
    Constant,
    Declaration,
    Function,
    Location,
    Node,
    Nodes,
    Operator,
    Program,
)
from editor.models.id import INVALID_ID, IDType

type _NodePair = tuple[IDType, Node]
type _DeclarationPair = tuple[IDType, Declaration]


class FileManager:
    """Handles access to FLUIR source files"""

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
        body: Nodes = {}
        for child in element.find("body").iterchildren():
            id, node = self._node(child)
            if id in body.keys():
                raise ValueError(f"{id} is repeated in his Declaration")
            body[id] = node
        return int(element.get("id")), Function(
            name=str(element.get("name")),
            id=int(element.get("id")),
            location=self._location(element),
            body=body,
        )

    def _node(self, element: Any) -> tuple[IDType, Node]:
        match element.tag:
            case tag if tag.endswith("binary"):
                return self._binary(element)
            case tag if tag.endswith("constant"):
                return self._constant(element)
        return (INVALID_ID, Constant())

    def _binary(self, element: Any) -> tuple[IDType, Node]:
        return int(element.get("id")), BinaryOperator(
            id=int(element.get("id")),
            location=self._location(element),
            op=Operator(element.get("operator")),
            lhs=int(element.get("lhs")),
            rhs=int(element.get("rhs")),
        )

    def _constant(self, element: Any) -> tuple[IDType, Node]:
        return int(element.get("id")), Constant(
            id=int(element.get("id")),
            location=self._location(element),
            value=self._value(next(element.iterchildren(), None)),
        )

    def _value(self, element: Any) -> float | None:
        match element.tag:
            case tag if tag.endswith("float"):
                return float(element.text)

        return None

    def _location(self, element: Any) -> Location:
        return Location(
            x=int(element.get("x")),
            y=int(element.get("y")),
            z=int(element.get("z")),
            width=int(element.get("w")),
            height=int(element.get("h")),
        )

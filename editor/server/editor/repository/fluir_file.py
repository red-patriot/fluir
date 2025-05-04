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
    UnaryOperator,
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
            if id == INVALID_ID:
                continue
            if id in declarations.keys():
                raise ValueError(f"{id} is repeated in the document")
            declarations[id] = decl

        return Program(declarations)

    def _declaration(self, element: Any) -> _DeclarationPair:
        body: Nodes = {}
        for child in element.find("body").iterchildren():
            id, node = self._node(child)
            if id == INVALID_ID:
                continue
            if id in body.keys():
                raise ValueError(f"{id} is repeated in his Declaration")
            body[id] = node
        return self._id(element), Function(
            name=str(element.get("name")),
            id=self._id(element),
            location=self._location(element),
            body=body,
        )

    def _node(self, element: Any) -> _NodePair:
        match element.tag:
            case tag if tag.endswith("binary"):
                return self._binary(element)
            case tag if tag.endswith("unary"):
                return self._unary(element)
            case tag if tag.endswith("constant"):
                return self._constant(element)
        return (INVALID_ID, Constant())

    def _binary(self, element: Any) -> _NodePair:
        return self._id(element), BinaryOperator(
            id=self._id(element),
            location=self._location(element),
            op=Operator(element.get("operator")),
            lhs=self._id(element, "lhs"),
            rhs=self._id(element, "rhs"),
        )

    def _unary(self, element: Any) -> _NodePair:
        return self._id(element), UnaryOperator(
            id=self._id(element),
            location=self._location(element),
            op=Operator(element.get("operator")),
            lhs=self._id(element, "lhs"),
        )

    def _constant(self, element: Any) -> _NodePair:
        return self._id(element), Constant(
            id=self._id(element),
            location=self._location(element),
            value=self._value(next(element.iterchildren(), None)),
        )

    def _value(self, element: Any) -> float | None:
        match element.tag:
            case tag if tag.endswith("float"):
                return float(element.text)

        return None

    def _id(self, element: Any, attribute: str = "id") -> IDType:
        return int(element.get(attribute))

    def _location(self, element: Any) -> Location:
        return Location(
            x=int(element.get("x")),
            y=int(element.get("y")),
            z=int(element.get("z")),
            width=int(element.get("w")),
            height=int(element.get("h")),
        )

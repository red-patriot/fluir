from pathlib import Path
from typing import Any, Final, cast, override

from lxml import etree
from lxml.objectify import ObjectifiedElement, fromstring

from editor.models import (
    INVALID_ID,
    BinaryOperator,
    Constant,
    Declaration,
    Function,
    IDType,
    Location,
    Node,
    Nodes,
    Operator,
    Program,
    UnaryOperator,
)
from editor.repository.interface.file_manager import FileManager

type _NodePair = tuple[IDType, Node]
type _DeclarationPair = tuple[IDType, Declaration]


class XMLFileManager(FileManager):
    """Handles access to XML  Fluir source files"""

    @override
    def parseStr(self, source: bytes) -> Program:
        root = fromstring(source)
        reader = _XMLReader()
        return reader.program(root)

    @override
    def parseFile(self, file: Path) -> Program:
        with open(file, "rb") as f:
            source = f.read()
        return self.parseStr(source)

    @override
    def writeFile(self, program: Program, file: Path) -> None:
        writer = _XMLWriter()
        contents = writer.write(program)
        with open(file, "wb") as f:
            f.write(contents)


class _XMLReader:
    """Reads an XML file into a Program"""

    def program(self, root: ObjectifiedElement) -> Program:
        declarations: list[Declaration] = []
        for element in root.iterchildren():
            id, decl = self._declaration(element)
            if id == INVALID_ID:
                continue
            declarations.append(decl)

        return Program(declarations)

    def _declaration(self, element: Any) -> _DeclarationPair:
        body: Nodes = []
        for child in element.find("body").iterchildren():
            id, node = self._node(child)
            if id == INVALID_ID:
                continue
            body.append(node)
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


_FL: Final[str] = "FLUIR::LANGUAGE::SOURCE"


class _XMLWriter:
    """Writes a program to XML"""

    def __init__(self) -> None:
        self.root = etree.Element("fluir", nsmap={"fl": _FL})

    def write(self, program: Program) -> bytes:
        for decl in program.declarations:
            self._decl(decl)

        return etree.tostring(
            self.root,
            pretty_print=True,
            xml_declaration=True,
            encoding="UTF-8",
        )

    def _decl(self, declaration: Declaration) -> None:
        decl_element = etree.SubElement(
            self.root,
            f"{{{_FL}}}function",
            attrib={
                "name": str(declaration.name),
                "id": str(declaration.id),
                "x": str(declaration.location.x),
                "y": str(declaration.location.y),
                "z": str(declaration.location.z),
                "w": str(declaration.location.width),
                "h": str(declaration.location.height),
            },
        )
        body_element = etree.SubElement(decl_element, "body")
        for node in declaration.body:
            self._node(node, body_element)

    def _node(self, node: Node, parent: etree._Element) -> None:
        match node._t:
            case "binary":
                self._binary(cast(BinaryOperator, node), parent)
            case "unary":
                self._unary(cast(UnaryOperator, node), parent)
            case "constant":
                self._constant(cast(Constant, node), parent)

    def _binary(self, node: BinaryOperator, parent: etree._Element) -> None:
        binary_element = etree.SubElement(
            parent,
            f"{{{_FL}}}binary",
            attrib={
                "id": str(node.id),
                "x": str(node.location.x),
                "y": str(node.location.y),
                "z": str(node.location.z),
                "w": str(node.location.width),
                "h": str(node.location.height),
                "lhs": str(node.lhs),
                "rhs": str(node.rhs),
                "operator": str(node.op),
            },
        )

    def _unary(self, node: UnaryOperator, parent: etree._Element) -> None:
        unary_element = etree.SubElement(
            parent,
            f"{{{_FL}}}unary",
            attrib={
                "id": str(node.id),
                "x": str(node.location.x),
                "y": str(node.location.y),
                "z": str(node.location.z),
                "w": str(node.location.width),
                "h": str(node.location.height),
                "lhs": str(node.lhs),
                "operator": str(node.op),
            },
        )

    def _constant(self, node: Constant, parent: etree._Element) -> None:
        constant_element = etree.SubElement(
            parent,
            f"{{{_FL}}}constant",
            attrib={
                "id": str(node.id),
                "x": str(node.location.x),
                "y": str(node.location.y),
                "z": str(node.location.z),
                "w": str(node.location.width),
                "h": str(node.location.height),
            },
        )
        float_element = etree.SubElement(
            constant_element,
            f"{{{_FL}}}float",
        )
        float_element.text = str(node.value)

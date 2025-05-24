from pathlib import Path
from typing import Any, assert_never, cast, override

from lxml import etree
from lxml.objectify import ObjectifiedElement, fromstring

from editor.models import (
    INVALID_ID,
    BinaryOperator,
    Conduit,
    Constant,
    Declaration,
    FlType,
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
type _ConduitPair = tuple[IDType, Conduit]
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
        nodes: Nodes = []
        conduits: list[Conduit] = []

        def handle_child(
            id_and_item: tuple[IDType, Any], items: list[Any]
        ) -> None:
            id, item = id_and_item
            if id == INVALID_ID:
                return
            items.append(item)

        for child in element.find("body").iterchildren():
            if child.tag == "conduit":
                handle_child(self._conduit(child), conduits)
            else:
                handle_child(self._node(child), nodes)
        return self._id(element), Function(
            name=str(element.get("name")),
            id=self._id(element),
            location=self._location(element),
            nodes=nodes,
            conduits=conduits,
        )

    def _node(self, element: Any) -> _NodePair:
        match element.tag:
            case "binary":
                return self._binary(element)
            case "unary":
                return self._unary(element)
            case "constant":
                return self._constant(element)
        return (INVALID_ID, Constant())

    def _conduit(self, element: Any) -> _ConduitPair:
        id = self._id(element)
        input = int(element.get("input"))
        children: list[Conduit.Segment | Conduit.Output] = []
        for child in element.iterchildren():
            if child.tag == "segment":
                children.append(self._conduit_segment(child))
            elif child.tag == "output":
                children.append(self._conduit_output(child))
            else:
                assert_never(child.tag)
        return id, Conduit(
            id=id,
            input=input,
            children=children,
        )

    def _conduit_segment(self, element: Any) -> Conduit.Segment:
        x = int(element.get("x"))
        y = int(element.get("y"))
        children: list[Conduit.Segment | Conduit.Output] = []
        for child in element.iterchildren():
            if child.tag == "segment":
                children.append(self._conduit_segment(child))
            elif child.tag == "output":
                children.append(self._conduit_output(child))
            else:
                assert_never(child.tag)
        return Conduit.Segment(x=x, y=y, children=children)

    def _conduit_output(self, element: Any) -> Conduit.Output:
        raw_target = element.text
        if raw_target is None:
            raise ValueError("Conduit output target is None")
        full_target = raw_target.split(":")
        return Conduit.Output(
            target=int(full_target[0]),
            index=full_target[1] if len(full_target) > 1 else 0,
        )

    def _binary(self, element: Any) -> _NodePair:
        return self._id(element), BinaryOperator(
            id=self._id(element),
            location=self._location(element),
            op=Operator(element.get("operator")),
        )

    def _unary(self, element: Any) -> _NodePair:
        return self._id(element), UnaryOperator(
            id=self._id(element),
            location=self._location(element),
            op=Operator(element.get("operator")),
        )

    def _constant(self, element: Any) -> _NodePair:
        return self._id(element), Constant(
            id=self._id(element),
            location=self._location(element),
            flType=self._type(next(element.iterchildren(), None)),
            value=self._value(next(element.iterchildren(), None)),
        )

    def _value(self, element: Any) -> str:
        return cast(str, element.text)

    def _type(self, element: Any) -> FlType | None:
        match element.tag:
            case "float":
                return FlType.FLOATING_POINT

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


class _XMLWriter:
    """Writes a program to XML"""

    def __init__(self) -> None:
        self.root = etree.Element("fluir")

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
            "function",
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
        for node in declaration.nodes:
            self._node(node, body_element)

    def _node(self, node: Node, parent: etree._Element) -> None:
        match node.discriminator:
            case "binary":
                self._binary(cast(BinaryOperator, node), parent)
            case "unary":
                self._unary(cast(UnaryOperator, node), parent)
            case "constant":
                self._constant(cast(Constant, node), parent)

    def _binary(self, node: BinaryOperator, parent: etree._Element) -> None:
        binary_element = etree.SubElement(
            parent,
            "binary",
            attrib={
                "id": str(node.id),
                "x": str(node.location.x),
                "y": str(node.location.y),
                "z": str(node.location.z),
                "w": str(node.location.width),
                "h": str(node.location.height),
                "operator": str(node.op),
            },
        )

    def _unary(self, node: UnaryOperator, parent: etree._Element) -> None:
        unary_element = etree.SubElement(
            parent,
            "unary",
            attrib={
                "id": str(node.id),
                "x": str(node.location.x),
                "y": str(node.location.y),
                "z": str(node.location.z),
                "w": str(node.location.width),
                "h": str(node.location.height),
                "operator": str(node.op),
            },
        )

    def _constant(self, node: Constant, parent: etree._Element) -> None:
        constant_element = etree.SubElement(
            parent,
            "constant",
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
            "float",
        )
        float_element.text = node.value

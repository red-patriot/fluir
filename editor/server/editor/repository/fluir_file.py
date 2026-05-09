from pathlib import Path
from typing import Any, assert_never, cast, override

from lxml import etree
from lxml.objectify import ObjectifiedElement, fromstring

from editor.models import (
    INVALID_ID,
    BinaryOperator,
    Call,
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
from editor.models.elements import (
    Annotation,
    Annotations,
    Comment,
    Header,
    Parameter,
    Return,
)
from editor.models.version import Version
from editor.repository.interface.file_manager import FileManager

type _NodePair = tuple[IDType, Node]
type _ConduitPair = tuple[IDType, Conduit]
type _DeclarationPair = tuple[IDType, Declaration]
type _AnnotationPair = tuple[IDType, Annotation]


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
        header: Header | None = None
        declarations: list[Declaration] = []
        annotations: Annotations = []
        for element in root.iterchildren():
            if element.tag == "header":
                header = self._header(element)
            elif element.tag == "function":
                id, decl = self._declaration(element)
                if id == INVALID_ID:
                    continue
                declarations.append(decl)
            elif element.tag == "comment":
                id, ann = self._annotation(element)
                if id == INVALID_ID:
                    continue
                annotations.append(ann)

        assert header is not None
        return Program(declarations, header, annotations)

    def _header(self, element: Any) -> Header:
        version_element = element.find("version")
        assert version_element is not None
        major = version_element.find("major")
        minor = version_element.find("minor")
        patch = version_element.find("patch")
        assert major is not None
        assert minor is not None
        assert patch is not None

        version = Version(
            MAJOR=int(major.text or "0"),
            MINOR=int(minor.text or "0"),
            PATCH=int(patch.text or "0"),
        )
        return Header(version=version)

    def _declaration(self, element: Any) -> _DeclarationPair:
        nodes: Nodes = []
        conduits: list[Conduit] = []
        annotations: Annotations = []
        input_block: list[Parameter] = []
        output_block: list[Return] = []

        input_element = element.find("input")
        if input_element is not None:
            input_block = self._input_block(input_element)

        output_element = element.find("output")
        if output_element is not None:
            output_block = self._output_block(output_element)

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
            elif child.tag == "comment":
                handle_child(self._annotation(child), annotations)
            else:
                handle_child(self._node(child), nodes)
        return self._id(element), Function(
            name=str(element.get("name")),
            id=self._id(element),
            location=self._location(element),
            nodes=nodes,
            conduits=conduits,
            inputs=input_block,
            outputs=output_block,
            annotations=annotations,
        )

    def _input_block(self, element: Any) -> list[Parameter]:
        params: list[Parameter] = []
        for child in element.iterchildren():
            if child.tag == "param":
                params.append(
                    Parameter(
                        name=str(child.get("name")),
                        id=self._id(child),
                        flType=self._type_from_attribute(child),
                    )
                )
        return params

    def _output_block(self, element: Any) -> list[Return]:
        returns: list[Return] = []
        for child in element.iterchildren():
            if child.tag == "return":
                returns.append(
                    Return(
                        id=self._id(child),
                        flType=self._type_from_attribute(child),
                    )
                )
        return returns

    def _type_from_attribute(self, element: Any) -> FlType | None:
        type_str: str | None = element.get("type")
        if type_str is None:
            return None
        return FlType(type_str)

    def _node(self, element: Any) -> _NodePair:
        match element.tag:
            case "binary":
                return self._binary(element)
            case "unary":
                return self._unary(element)
            case "constant":
                return self._constant(element)
            case "call":
                return self._call(element)
        return (INVALID_ID, Constant())

    def _annotation(self, element: Any) -> _AnnotationPair:
        match element.tag:
            case "comment":
                return self._comment(element)
        return (INVALID_ID, Comment())

    def _comment(self, element: Any) -> _AnnotationPair:
        id = self._id(element)
        return id, Comment(
            id=id,
            location=self._location(element),
            data=element.text or "",
        )

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
        target = int(element.get("target"))
        index = int(element.get("index")) if "index" in element.keys() else 0
        return Conduit.Output(
            target=target,
            index=index,
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

    def _call(self, element: Any) -> _NodePair:
        id = self._id(element)
        indexed_args: list[tuple[int, str]] = []
        returns = False
        for child in element.iterchildren():
            if child.tag == "arg":
                indexed_args.append(
                    (int(child.get("index")), str(child.get("name")))
                )
            elif child.tag == "return":
                # TODO: Handle multiple returns
                returns = True
        indexed_args.sort(key=lambda pair: pair[0])
        arguments = [name for _, name in indexed_args]
        return id, Call(
            id=id,
            location=self._location(element),
            target=str(element.get("target")),
            arguments=arguments,
            returns=returns,
        )

    def _value(self, element: Any) -> str:
        return cast(str, element.text)

    def _type(self, element: Any) -> FlType | None:
        match element.tag:
            case "f64":
                return FlType.F64
            case "i8":
                return FlType.I8
            case "i16":
                return FlType.I16
            case "i32":
                return FlType.I32
            case "i64":
                return FlType.I64
            case "u8":
                return FlType.U8
            case "u16":
                return FlType.U16
            case "u32":
                return FlType.U32
            case "u64":
                return FlType.U64

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
        self._header(program.header)
        for decl in program.declarations:
            self._decl(decl)
        for annotation in program.annotations:
            self._annotation(annotation, self.root)

        return etree.tostring(
            self.root,
            pretty_print=True,
            xml_declaration=True,
            encoding="UTF-8",
        )

    def _header(self, header: Header) -> None:
        header_element = etree.SubElement(self.root, "header")
        self._version(header_element, header.version)

    def _version(self, parent: etree._Element, version: Version) -> None:
        version_element = etree.SubElement(parent, "version")

        major_element = etree.SubElement(version_element, "major")
        major_element.text = str(version.MAJOR)

        minor_element = etree.SubElement(version_element, "minor")
        minor_element.text = str(version.MINOR)

        patch_element = etree.SubElement(version_element, "patch")
        patch_element.text = str(version.PATCH)

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
        if declaration.inputs:
            self._input_block(declaration.inputs, decl_element)
        if declaration.outputs:
            self._output_block(declaration.outputs, decl_element)
        body_element = etree.SubElement(decl_element, "body")
        for node in declaration.nodes:
            self._node(node, body_element)
        for conduit in declaration.conduits:
            self._conduit(conduit, body_element)
        for annotation in declaration.annotations:
            self._annotation(annotation, body_element)

    def _input_block(
        self, inputs: list[Parameter], parent: etree._Element
    ) -> None:
        input_element = etree.SubElement(
            parent,
            "input",
        )
        for param in inputs:
            attrib: dict[str, str] = {
                "name": param.name,
                "id": str(param.id),
            }
            if param.flType is not None:
                attrib["type"] = str(param.flType)
            etree.SubElement(input_element, "param", attrib=attrib)

    def _output_block(
        self, outputs: list[Return], parent: etree._Element
    ) -> None:
        output_element = etree.SubElement(
            parent,
            "output",
        )
        for ret in outputs:
            attrib: dict[str, str] = {
                "id": str(ret.id),
            }
            if ret.flType is not None:
                attrib["type"] = str(ret.flType)
            etree.SubElement(output_element, "return", attrib=attrib)

    def _node(self, node: Node, parent: etree._Element) -> None:
        match node.discriminator:
            case "binary":
                self._binary(cast(BinaryOperator, node), parent)
            case "unary":
                self._unary(cast(UnaryOperator, node), parent)
            case "constant":
                self._constant(cast(Constant, node), parent)
            case "call":
                self._call(cast(Call, node), parent)

    def _annotation(
        self, annotation: Annotation, parent: etree._Element
    ) -> None:
        match annotation.discriminator:
            case "comment":
                self._comment(annotation, parent)

    def _comment(self, comment: Comment, parent: etree._Element) -> None:
        comment_element = etree.SubElement(
            parent,
            "comment",
            attrib={
                "id": str(comment.id),
                "x": str(comment.location.x),
                "y": str(comment.location.y),
                "z": str(comment.location.z),
                "w": str(comment.location.width),
                "h": str(comment.location.height),
            },
        )
        comment_element.text = comment.data

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
        assert node.flType is not None
        assert node.value is not None
        self._literal(node.flType, node.value, constant_element)

    def _call(self, node: Call, parent: etree._Element) -> None:
        call_element = etree.SubElement(
            parent,
            "call",
            attrib={
                "target": node.target,
                "id": str(node.id),
                "x": str(node.location.x),
                "y": str(node.location.y),
                "z": str(node.location.z),
                "w": str(node.location.width),
                "h": str(node.location.height),
            },
        )
        if node.returns:
            etree.SubElement(call_element, "return")
        for name in node.arguments:
            etree.SubElement(
                call_element,
                "arg",
                attrib={"name": name},
            )

    def _literal(
        self, type_: FlType, value: str, parent: etree._Element
    ) -> None:
        literal_element = etree.SubElement(parent, str(type_).lower())
        literal_element.text = value

    def _conduit(self, conduit: Conduit, parent: etree._Element) -> None:
        conduit_element = etree.SubElement(
            parent,
            "conduit",
            attrib={
                "id": str(conduit.id),
                "input": str(conduit.input),
            },
        )
        for child in conduit.children:
            if isinstance(child, Conduit.Segment):
                self._conduit_segment(child, conduit_element)
            elif isinstance(child, Conduit.Output):
                self._conduit_output(child, conduit_element)

    def _conduit_segment(
        self, segment: Conduit.Segment, parent: etree._Element
    ) -> None:
        segment_element = etree.SubElement(
            parent,
            "segment",
            attrib={
                "x": str(segment.x),
                "y": str(segment.y),
            },
        )
        for child in segment.children:
            if isinstance(child, Conduit.Segment):
                self._conduit_segment(child, segment_element)
            elif isinstance(child, Conduit.Output):
                self._conduit_output(child, segment_element)

    def _conduit_output(
        self, output: Conduit.Output, parent: etree._Element
    ) -> None:
        output_element = etree.SubElement(parent, "output")
        output_element.set("target", str(output.target))
        output_element.set("index", str(output.index))

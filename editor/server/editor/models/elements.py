from dataclasses import field
from enum import StrEnum
from typing import Literal, Never, Sequence, cast

from pydantic.dataclasses import dataclass

from editor.models.id import INVALID_ID, IDType, QualifiedID
from editor.models.version import FLUIR_CURRENT_VERSION, Version


class IdentifierError(Exception):
    pass


class FlType(StrEnum):
    F64 = "F64"
    I8 = "I8"
    I16 = "I16"
    I32 = "I32"
    I64 = "I64"
    U8 = "U8"
    U16 = "U16"
    U32 = "U32"
    U64 = "U64"
    BOOL = "BOOL"


class Operator(StrEnum):
    UNKNOWN = " "
    PLUS = "+"
    MINUS = "-"
    STAR = "*"
    SLASH = "/"
    PLUS_PLUS = "++"
    MINUS_MINUS = "--"


@dataclass
class Location:
    x: int = 0
    y: int = 0
    z: int = 0
    width: int = 0
    height: int = 0


@dataclass
class Constant:
    discriminator: Literal["constant"] = "constant"
    id: IDType = INVALID_ID
    location: Location = field(default_factory=Location)
    flType: FlType | None = None
    value: str | None = None


@dataclass
class BinaryOperator:
    discriminator: Literal["binary"] = "binary"
    id: IDType = INVALID_ID
    location: Location = field(default_factory=Location)
    op: Operator = Operator.UNKNOWN


@dataclass
class UnaryOperator:
    discriminator: Literal["unary"] = "unary"
    id: IDType = INVALID_ID
    location: Location = field(default_factory=Location)
    op: Operator = Operator.UNKNOWN


@dataclass
class Call:
    discriminator: Literal["call"] = "call"
    target: str = ""
    id: IDType = INVALID_ID
    location: Location = field(default_factory=Location)
    arguments: list[str] = field(default_factory=list)
    returns: bool = False  # TODO: For now, indicates if there is a return value, in the future, add multiple return values


@dataclass
class Comment:
    discriminator: Literal["comment"] = "comment"
    id: IDType = INVALID_ID
    location: Location = field(default_factory=Location)
    data: str = ""


Node = Constant | BinaryOperator | UnaryOperator | Call
Nodes = list[Node]


Annotation = Comment
Annotations = list[Annotation]


@dataclass
class Conduit:
    @dataclass
    class Output:
        discriminator: Literal["conduit_output"] = "conduit_output"
        target: IDType = INVALID_ID
        index: int = 0

    @dataclass
    class Segment:
        discriminator: Literal["conduit_segment"] = "conduit_segment"
        x: int = 0
        y: int = 0
        children: list["Conduit.Segment | Conduit.Output"] = field(
            default_factory=list
        )

    id: IDType = INVALID_ID
    input: IDType = INVALID_ID
    index: int = 0
    children: list[Segment | Output] = field(default_factory=list)


@dataclass
class Parameter:
    name: str = ""
    id: IDType = INVALID_ID
    flType: FlType | None = None


@dataclass
class Return:
    id: IDType = INVALID_ID
    flType: FlType | None = None


@dataclass
class Function:
    discriminator: Literal["function"] = "function"
    name: str = ""
    id: IDType = INVALID_ID
    location: Location = field(default_factory=Location)
    nodes: Nodes = field(default_factory=list)
    conduits: list[Conduit] = field(default_factory=list)
    inputs: list[Parameter] = field(default_factory=list)
    outputs: list[Return] = field(default_factory=list)
    annotations: Annotations = field(default_factory=list)


Declaration = Function
Declarations = list[Declaration]


@dataclass
class Header:
    version: Version = field(default_factory=lambda: FLUIR_CURRENT_VERSION)


@dataclass
class Program:
    declarations: Declarations = field(default_factory=list)
    header: Header = field(default_factory=Header)
    annotations: Annotations = field(default_factory=list)


Element = Declaration | Node | Annotation
Item = Declaration | Node | Conduit | Annotation


def _find_impl(
    id: QualifiedID, elements: Sequence[Element | Conduit]
) -> Item | None:
    first = id[0]
    for element in elements:
        if element.id == first:
            if len(id) == 1:
                return element
            if isinstance(element, Conduit):
                return None
            match element.discriminator:
                case "function":
                    func = cast(Function, element)
                    return (
                        _find_impl(id[1:], func.nodes)
                        or _find_impl(id[1:], func.conduits)
                        or _find_impl(id[1:], func.annotations)
                    )
                case "binary":
                    return None
                case "unary":
                    return None
                case "constant":
                    return None
                case "call":
                    return None
                case "comment":
                    return None
                case _:
                    assert Never
    return None


def find_element(id: QualifiedID, program: Program) -> Element:
    """Find the element with the given ID in the program."""
    if len(id) == 0 or INVALID_ID in id:
        raise IdentifierError("ID is invalid")
    found = _find_impl(id, program.declarations) or _find_impl(
        id, program.annotations
    )
    if found is not None and not isinstance(found, Conduit):
        return found
    raise IdentifierError(
        f"An element with ID {':'.join(map(str, id))} was not found"
    )


def find_item(id: QualifiedID, program: Program) -> Item:
    """Find the element or conduit with the given ID in the program."""
    if len(id) == 0 or INVALID_ID in id:
        raise IdentifierError("ID is invalid")
    found = _find_impl(id, program.declarations) or _find_impl(
        id, program.annotations
    )
    if found is not None:
        return found
    raise IdentifierError(
        f"An element with ID {':'.join(map(str, id))} was not found"
    )

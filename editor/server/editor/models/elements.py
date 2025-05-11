from dataclasses import field
from enum import StrEnum
from typing import Literal, Never, Sequence, cast

from pydantic.dataclasses import dataclass

from editor.models.id import INVALID_ID, IDType, QualifiedID


class IdentifierError(Exception):
    pass


class Operator(StrEnum):
    UNKNOWN = "<?>"
    PLUS = "+"
    MINUS = "-"
    STAR = "*"
    SLASH = "/"


@dataclass
class Location:
    x: int = 0
    y: int = 0
    z: int = 0
    width: int = 0
    height: int = 0


@dataclass
class Constant:
    _t: Literal["constant"] = "constant"
    id: IDType = INVALID_ID
    location: Location = field(default_factory=Location)
    value: float | None = None


@dataclass
class BinaryOperator:
    _t: Literal["binary"] = "binary"
    id: IDType = INVALID_ID
    location: Location = field(default_factory=Location)
    op: Operator = Operator.UNKNOWN
    lhs: IDType = INVALID_ID
    rhs: IDType = INVALID_ID


@dataclass
class UnaryOperator:
    _t: Literal["unary"] = "unary"
    id: IDType = INVALID_ID
    location: Location = field(default_factory=Location)
    op: Operator = Operator.UNKNOWN
    lhs: IDType = INVALID_ID


type Node = Constant | BinaryOperator | UnaryOperator
type Nodes = list[Node]


@dataclass
class Function:
    _t: Literal["function"] = "function"
    name: str = ""
    id: IDType = INVALID_ID
    location: Location = field(default_factory=Location)
    body: Nodes = field(default_factory=list)


type Declaration = Function
type Declarations = list[Declaration]


@dataclass
class Program:
    declarations: Declarations = field(default_factory=list)


type Element = Declaration | Node


def _find_impl(id: QualifiedID, elements: Sequence[Element]) -> Element | None:
    first = id[0]
    for element in elements:
        if element.id == first:
            if len(id) == 1:
                return element
            match element._t:
                case "function":
                    return _find_impl(id[1:], cast(Function, element).body)
                case "binary":
                    return None
                case "unary":
                    return None
                case "constant":
                    return None
                case _:
                    return Never
    return None


def find_element(id: QualifiedID, program: Program) -> Element:
    if len(id) == 0 or INVALID_ID in id:
        raise IdentifierError("ID is invalid")
    found = _find_impl(id, program.declarations)
    if found is not None:
        return found
    raise IdentifierError(
        f"An element with ID {':'.join(map(str, id))} was not found"
    )

from dataclasses import field
from enum import StrEnum
from typing import Literal

from pydantic.dataclasses import dataclass

from editor.models.id import INVALID_ID, IDType


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

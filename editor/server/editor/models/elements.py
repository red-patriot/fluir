from dataclasses import field
from enum import StrEnum

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
    id: IDType = INVALID_ID
    location: Location = field(default_factory=Location)
    value: float | None = None


@dataclass
class BinaryOperator:
    id: IDType = INVALID_ID
    location: Location = field(default_factory=Location)
    op: Operator = Operator.UNKNOWN
    lhs: IDType = INVALID_ID
    rhs: IDType = INVALID_ID


@dataclass
class UnaryOperator:
    id: IDType = INVALID_ID
    location: Location = field(default_factory=Location)
    op: Operator = Operator.UNKNOWN
    lhs: IDType = INVALID_ID


type Node = Constant | BinaryOperator | UnaryOperator
type Nodes = dict[IDType, Node]


@dataclass
class Function:
    name: str = ""
    id: IDType = INVALID_ID
    location: Location = field(default_factory=Location)
    body: dict[IDType, Node] = field(default_factory=dict)


type Declaration = Function
type Declarations = dict[IDType, Declaration]


@dataclass
class Program:
    declarations: Declarations = field(default_factory=dict)

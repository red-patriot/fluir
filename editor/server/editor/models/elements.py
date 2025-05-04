from dataclasses import dataclass, field
from typing import Any, Final

type IDType = int
INVALID_ID: Final[IDType] = 0


@dataclass
class Location:
    x: int = 0
    y: int = 0
    z: int = 0
    width: int = 0
    height: int = 0


@dataclass
class Function:
    name: str = ""
    id: int = INVALID_ID
    location: Location = field(default_factory=Location)
    body: dict[Any, Any] = field(default_factory=dict)


type Declaration = Function
type Declarations = dict[IDType, Declaration]


@dataclass
class Program:
    declarations: Declarations = field(default_factory=dict)

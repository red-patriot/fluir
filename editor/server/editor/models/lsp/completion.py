from enum import StrEnum
from typing import Literal, Union

from pydantic import BaseModel
from pydantic.dataclasses import dataclass

from editor.models import FlType


class Kind(StrEnum):
    """The kind of a completion"""

    FUNCTION_DEF = "function"
    CALL = "call"
    CONSTANT = "constant"
    OPERATOR = "operator"
    COMMENT = "comment"


class ConstantData(BaseModel):
    kind: Literal[Kind.CONSTANT] = Kind.CONSTANT
    value: str = ""
    flType: FlType | None = None


class OperatorData(BaseModel):
    kind: Literal[Kind.OPERATOR] = Kind.OPERATOR
    arity: Literal[1, 2]


class NoData(BaseModel):
    kind: Kind


@dataclass
class Completion:
    """A completion option for adding a new element"""

    data: ConstantData | OperatorData | NoData
    short_name: str  # Short one-line description of the completion
    description: str = ""  # Complete description of the completion item

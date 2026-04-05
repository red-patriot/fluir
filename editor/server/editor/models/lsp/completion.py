from enum import StrEnum

from pydantic.dataclasses import dataclass


class Kind(StrEnum):
    """The kind of a completion"""

    FUNCTION_DEF = "function"
    CALL = "call"
    CONSTANT = "constant"
    OPERATOR = "operator"


@dataclass
class Completion:
    """A completion option for adding a new element"""

    short_name: str  # Short one-line description of the completion
    kind: Kind  # The kind of completion this represents
    description: str = ""  # Complete description of the completion item

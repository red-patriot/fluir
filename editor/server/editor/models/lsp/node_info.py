from pydantic.dataclasses import dataclass

from editor.models.elements import FlType


@dataclass
class NodeInput:
    """Information about the input to a node"""

    expected_type: FlType
    provided_type: FlType | None


@dataclass
class NodeInfo:
    """Information about a node on the flow diagram"""

    label: str
    description: str | None = None
    inputs: list[NodeInput] | None = None
    outputs: list[FlType] | None = None

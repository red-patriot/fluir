from dataclasses import dataclass, field


@dataclass
class ParamInfo:
    name: str = ""
    flType: str = ""


@dataclass
class FunctionSignature:
    """A function type signature with input and output types"""

    inputs: list[ParamInfo] = field(default_factory=list)
    # TODO: Support multiple returns
    output: str | None = None

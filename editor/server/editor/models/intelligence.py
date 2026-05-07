from dataclasses import dataclass, field


@dataclass
class FunctionSignature:
    """A function type signature with input and output types"""

    inputs: list[str] = field(default_factory=list)
    # TODO: Support multiple returns
    output: str | None = None

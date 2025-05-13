from pydantic.dataclasses import dataclass

from editor.models import Program


@dataclass
class ProgramStatus:
    """The overall status of the open program"""

    saved: bool
    path: str
    program: Program

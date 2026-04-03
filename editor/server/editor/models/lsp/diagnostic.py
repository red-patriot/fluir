from enum import StrEnum

from pydantic.dataclasses import dataclass

from editor.models.id import QualifiedID


class Severity(StrEnum):
    NOTE = "NOTE"
    WARNING = "WARNING"
    ERROR = "ERROR"


@dataclass
class Diagnostic:
    Severity: Severity
    text: str
    where: QualifiedID

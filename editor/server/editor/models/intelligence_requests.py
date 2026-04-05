from pydantic.dataclasses import dataclass

from editor.models.id import QualifiedID


@dataclass
class CompletionRequest:
    block_id: QualifiedID
    path: str

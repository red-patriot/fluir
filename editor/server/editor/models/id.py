from typing import Final

type IDType = int
type QualifiedID = list[IDType]

INVALID_ID: Final[IDType] = 0


def make_qualified_id(string: str) -> QualifiedID:
    """Create a qualified ID from n ID string."""
    return [int(part) for part in string.split(":")]

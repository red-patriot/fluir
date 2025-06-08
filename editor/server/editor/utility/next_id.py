from editor.models import elements
from editor.models.id import IDType


def next_id(decl: elements.Declaration) -> IDType:
    """Get the next available ID for a given declaration's children."""
    match type(decl):
        case elements.Function:
            max_node_id = max((n.id for n in decl.nodes), default=0)
            max_conduit_id = max((c.id for c in decl.conduits), default=0)
            return max(max_node_id, max_conduit_id) + 1
        case _:
            raise TypeError(f"Unsupported declaration type: {type(decl)}")

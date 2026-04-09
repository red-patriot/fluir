from editor.models import elements
from editor.models.id import IDType


def next_id(decl: elements.Declaration) -> IDType:
    """Get the next available ID for a given declaration's children."""
    match type(decl):
        case elements.Function:
            max_node_id = max((n.id for n in decl.nodes), default=0)
            max_conduit_id = max((c.id for c in decl.conduits), default=0)
            max_input_id = (
                max((p.id for p in decl.input.elements), default=0)
                if decl.input is not None
                else 0
            )
            max_output_id = (
                max((r.id for r in decl.output.elements), default=0)
                if decl.output is not None
                else 0
            )
            # TODO: Use some better way of generating IDs here?
            return (
                max((max_node_id, max_conduit_id, max_input_id, max_output_id))
                + 1
            )
        case _:
            raise TypeError(f"Unsupported declaration type: {type(decl)}")


def next_decl_id(program: elements.Program) -> IDType:
    max_id = max((decl.id for decl in program.declarations), default=0)
    return max(max_id, 0) + 1

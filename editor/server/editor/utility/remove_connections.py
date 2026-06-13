from dataclasses import dataclass
from typing import cast

from editor.models import elements


@dataclass
class ConnectionTarget:
    id: int
    index: int | None = None


def remove_connections(
    target: ConnectionTarget, parent: elements.Function
) -> list[elements.Conduit]:
    """
    Removes conduits connecting to the given target in the given program.
    Returns the conduits that were removed.
    """
    ret: list[elements.Conduit] = []
    for conduit in parent.conduits:
        if conduit.input == target.id and (
            target.index is None or conduit.index == target.index
        ):
            # Remove conduits coming out of this target
            ret.append(conduit)
            parent.conduits.remove(conduit)
        elif any(
            (
                # TODO: Support nested segments too
                cast(elements.Conduit.Output, out).target == target.id
                and (
                    target.index is None
                    or cast(elements.Conduit.Output, out).index == target.index
                )
            )
            for out in conduit.children
        ):
            # Remove conduits going into this target
            ret.append(conduit)
            parent.conduits.remove(conduit)
    return ret

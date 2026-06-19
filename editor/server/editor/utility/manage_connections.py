from dataclasses import dataclass
from typing import cast

from editor.models import IDType, elements


@dataclass
class ConnectionTarget:
    id: IDType
    index: int | None = None


def _is_input(target: ConnectionTarget, conduit: elements.Conduit) -> bool:
    return any(
        (
            # TODO: Support nested segments too
            cast(elements.Conduit.Output, out).target == target.id
            and (
                target.index is None
                or cast(elements.Conduit.Output, out).index == target.index
            )
        )
        for out in conduit.children
    )


def _is_output(target: ConnectionTarget, conduit: elements.Conduit) -> bool:
    return conduit.input == target.id and (
        target.index is None or conduit.index == target.index
    )


def remove_inputs_to(
    target: ConnectionTarget, parent: elements.Function
) -> list[elements.Conduit]:
    """
    Removes conduits connecting to the given target in the given program.
    Returns the conduits that were removed.
    """
    ret: list[elements.Conduit] = []
    for conduit in parent.conduits:
        if _is_input(target, conduit):
            ret.append(conduit)
            parent.conduits.remove(conduit)
    return ret


def remove_outputs_of(
    target: ConnectionTarget, parent: elements.Function
) -> list[elements.Conduit]:
    """
    Removes conduits connecting to the given target in the given program.
    Returns the conduits that were removed.
    """
    ret: list[elements.Conduit] = []
    for conduit in parent.conduits:
        if _is_output(target, conduit):
            ret.append(conduit)
            parent.conduits.remove(conduit)
    return ret


def remove_all_connections_of(
    target: ConnectionTarget, parent: elements.Function
) -> list[elements.Conduit]:
    """
    Removes conduits connecting to the given target in the given program.
    Returns the conduits that were removed.
    """
    ret: list[elements.Conduit] = []
    for conduit in parent.conduits:
        if _is_input(target, conduit) or _is_output(target, conduit):
            # Remove conduits coming out of this target
            ret.append(conduit)
            parent.conduits.remove(conduit)
    return ret


def rotate_input_indices(
    source: int,
    destination: int,
    target: IDType,
    parent: elements.Function,
) -> None:
    """
    Rotates conduits going into `target`.
    Moves all conduits between index `source` and index `destination` up
    or down. (see diagram for example)

    Case 1:     1 -> 3     |
                           |
                 |call|    |                 |call|
    |c0>-------->| p0 |    |    |c0>-------->| p0 |
    |c1>-------->| p1 |    |    |c2>-------->| p2 |
    |c2>-------->| p2 |    |    |c3>-------->| p3 |
    |c3>-------->| p3 |    |    |c1>-------->| p1 |
    |c4>-------->| p4 |    |    |c4>-------->| p4 |

    Case 2:     3 -> 1     |
                           |
                 |call|    |                 |call|
    |c0>-------->| p0 |    |    |c0>-------->| p0 |
    |c1>-------->| p1 |    |    |c3>-------->| p3 |
    |c2>-------->| p2 |    |    |c1>-------->| p1 |
    |c3>-------->| p3 |    |    |c2>-------->| p2 |
    |c4>-------->| p4 |    |    |c4>-------->| p4 |

    """
    first = min(source, destination)
    last = max(source, destination)

    # Gather conduits into original indices
    def outputsInRange(conduit: elements.Conduit) -> bool:
        # TODO: handle other children types
        out = cast(elements.Conduit.Output, conduit.children[0])
        return out.target == target and first <= out.index <= last

    def idxOf(conduit: elements.Conduit) -> int:
        return cast(elements.Conduit.Output, conduit.children[0]).index

    conduits = {
        idxOf(conduit): conduit
        for conduit in parent.conduits
        if outputsInRange(conduit)
    }

    # Rotate each in the correct direction
    direction = -1 if source < destination else +1
    for idx, conduit in conduits.items():
        out = cast(elements.Conduit.Output, conduit.children[0])
        out.index = idx + direction

    # Patch source -> destination
    if source in conduits:
        out = cast(elements.Conduit.Output, conduits[source].children[0])
        out.index = destination

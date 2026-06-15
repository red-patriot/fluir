from dataclasses import dataclass
from typing import cast

from editor.models import elements


@dataclass
class ConnectionTarget:
    id: int
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

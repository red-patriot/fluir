from dataclasses import dataclass
from typing import Literal, override

from pydantic import BaseModel

from editor.models import Program, QualifiedID, elements
from editor.models.elements import find_element
from editor.services.transaction.base import TransactionBase


@dataclass
class _Limits:
    upper: int
    lower: int


class ResizeElement(BaseModel, TransactionBase):
    discriminator: Literal["resize"] = "resize"
    target: QualifiedID
    width: int
    height: int
    x: int | None = None
    y: int | None = None

    @override
    def do(self, original: Program) -> Program:
        element = find_element(self.target, original)
        limits = self._get_limits(original, element)
        if limits:
            horizontal, vertical = limits
            self.width = max(
                horizontal.lower,
                min(self.width, horizontal.upper - element.location.x),
            )
            self.height = max(
                vertical.lower,
                min(self.height, vertical.upper - element.location.y),
            )
        new_width, self.width = self.width, element.location.width
        new_height, self.height = self.height, element.location.height

        element.location.width = new_width
        element.location.height = new_height
        if self.x is not None:
            element.location.x, self.x = self.x, element.location.x
        if self.y is not None:
            element.location.y, self.y = self.y, element.location.y
        return original

    @override
    def undo(self, original: Program) -> Program:
        element = find_element(self.target, original)
        new_width, self.width = self.width, element.location.width
        new_height, self.height = self.height, element.location.height

        element.location.width = new_width
        element.location.height = new_height
        if self.x is not None:
            element.location.x, self.x = self.x, element.location.x
        if self.y is not None:
            element.location.y, self.y = self.y, element.location.y
        return original

    def _get_limits(
        self, program: Program, element: elements.Element
    ) -> tuple[_Limits, _Limits] | None:
        ret = (
            _Limits(upper=1000, lower=4),
            _Limits(upper=1000, lower=4),
        )

        if isinstance(element, elements.Declaration):
            # TODO: Handle other declarations
            ret[0].lower = max(
                (
                    node.location.x + node.location.width
                    for node in element.nodes
                ),
                default=15,
            )
            ret[1].lower = max(
                (
                    node.location.y + node.location.height
                    for node in element.nodes
                ),
                default=15,
            )
        if len(self.target) > 1:
            enclosing = find_element(self.target[:-1], program)
            ret[0].upper = enclosing.location.width
            ret[1].upper = enclosing.location.height
        return ret

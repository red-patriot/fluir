from typing import cast, override

from pydantic import BaseModel

from editor.models import Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import find_item
from editor.services.transaction.base import TransactionBase


class RemoveItem(BaseModel, TransactionBase):
    target: QualifiedID
    _removed_item: elements.Item | None = None
    _removed_conduits: list[elements.Conduit] = []

    @override
    def do(self, original: Program) -> Program:
        if len(self.target) < 1:
            raise BadEdit("Cannot remove the program itself")

        if len(self.target) == 1:
            # Removing a function from the program
            # Find and store the function being removed
            for func in original.declarations:
                if func.id == self.target[0]:
                    self._removed_item = func
                    break

            if self._removed_item is None:
                raise BadEdit(f"Function {self.target[0]} not found")

            original.declarations = [
                func
                for func in original.declarations
                if func.id != self.target[0]
            ]
            return original

        parent_id = self.target[:-1]
        parent = find_item(parent_id, original)
        element_id = self.target[-1]

        if isinstance(parent, elements.Function):
            # Find and store the item being removed
            self._removed_item = None

            # Check if it's a node
            for node in parent.nodes:
                if node.id == element_id:
                    self._removed_item = node
                    break

            # Check if it's a conduit
            if self._removed_item is None:
                for conduit in parent.conduits:
                    if conduit.id == element_id:
                        self._removed_item = conduit
                        break

            if self._removed_item is None:
                raise BadEdit(f"Element {element_id} not found")

            # Store conduits that will be removed due to connections
            self._removed_conduits = []
            for conduit in parent.conduits:
                if conduit.input == element_id or any(
                    cast(elements.Conduit.Output, out).target == element_id
                    for out in conduit.children
                ):
                    # Only store if it's not the item being directly removed
                    if conduit.id != element_id:
                        self._removed_conduits.append(conduit)

            # Remove from nodes list
            parent.nodes = [
                node for node in parent.nodes if node.id != element_id
            ]
            parent.conduits = [
                conduit
                for conduit in parent.conduits
                if conduit.id != element_id
            ]
            # Remove any conduits connected to this node
            parent.conduits = [
                conduit
                for conduit in parent.conduits
                if conduit.input != element_id
                and not any(
                    cast(elements.Conduit.Output, out).target == element_id
                    for out in conduit.children
                )
            ]
        else:
            raise BadEdit("Can only remove elements from functions")

        return original

    @override
    def undo(self, original: Program) -> Program:
        if self._removed_item is None:
            raise BadEdit("Nothing to undo - no item was removed")

        if len(self.target) == 1:
            # Restoring a function to the program
            assert isinstance(self._removed_item, elements.Function)
            original.declarations.append(self._removed_item)
            return original

        parent_id = self.target[:-1]
        parent = find_item(parent_id, original)

        if isinstance(parent, elements.Function):
            # Restore the removed item
            if isinstance(self._removed_item, elements.Node):
                parent.nodes.append(self._removed_item)
            elif isinstance(self._removed_item, elements.Conduit):
                parent.conduits.append(self._removed_item)

            # Restore any conduits that were removed due to connections
            for conduit in self._removed_conduits:
                parent.conduits.append(conduit)
        else:
            raise BadEdit("Can only restore elements to functions")
        self._removed_conduits = []
        self._removed_item = None

        return original

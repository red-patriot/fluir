from typing import Literal, override

from pydantic import BaseModel

from editor.models import Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import find_element
from editor.models.id import IDType
from editor.services.transaction.base import TransactionBase
from editor.services.transaction.remove import RemoveItem
from editor.utility.next_id import next_decl_id, next_id


class AddComment(BaseModel, TransactionBase):
    discriminator: Literal["add_comment"] = "add_comment"
    parent: QualifiedID
    new_location: elements.Location
    data: str = ""
    _inserted: IDType | None = None

    @override
    def do(self, original: Program) -> Program:
        if len(self.parent) == 0:
            new_id = next_decl_id(original)
            original.annotations.append(
                elements.Comment(
                    id=new_id,
                    location=self.new_location,
                    data=self.data,
                )
            )
            self._inserted = new_id
            return original

        decl = find_element(self.parent, original)
        if not isinstance(decl, elements.Function):
            raise BadEdit("Parent of a comment must be a function")
        new_id = next_id(decl)
        decl.annotations.append(
            elements.Comment(
                id=new_id,
                location=self.new_location,
                data=self.data,
            )
        )
        self._inserted = new_id
        return original

    @override
    def undo(self, original: Program) -> Program:
        assert self._inserted is not None
        return RemoveItem(target=self.parent + [self._inserted]).do(original)

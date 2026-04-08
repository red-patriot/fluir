from typing import Literal, override

from pydantic import BaseModel

from editor.models import Program, elements
from editor.models.id import IDType
from editor.services.transaction.base import TransactionBase
from editor.services.transaction.remove import RemoveItem
from editor.utility.next_id import next_decl_id


class FunctionParams(BaseModel):
    discriminator: Literal["function"] = "function"
    name: str | None = None


class AddDecl(BaseModel, TransactionBase):
    discriminator: Literal["add_decl"] = "add_decl"
    new_location: elements.Location
    params: FunctionParams = FunctionParams()
    _inserted: IDType | None = None

    @override
    def do(self, original: Program) -> Program:
        new_id = next_decl_id(original)
        name = (
            self.params.name if self.params.name is not None else "new_function"
        )
        original.declarations.append(
            elements.Function(
                id=new_id,
                name=name,
                location=self.new_location,
            )
        )
        self._inserted = new_id
        return original

    @override
    def undo(self, original: Program) -> Program:
        assert self._inserted is not None
        return RemoveItem(target=[self._inserted]).do(original)

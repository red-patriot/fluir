from typing import Literal, override

from pydantic import BaseModel

from editor.models import FlType, IDType, Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import find_element
from editor.services.transaction.base import TransactionBase
from editor.services.transaction.remove import RemoveItem
from editor.utility.next_id import next_id


class DeclParameterParams(BaseModel):
    discriminator: Literal["parameter"] = "parameter"
    name: str


class DeclReturnParams(BaseModel):
    discriminator: Literal["return"] = "return"


class AddDeclInterface(BaseModel, TransactionBase):
    """Add an interface element (parameter, return, capture) to a function decl"""

    discriminator: Literal["add_decl_interface"] = "add_decl_interface"
    parent: QualifiedID
    flType: FlType
    params: DeclParameterParams | DeclReturnParams
    _added: IDType | None = None

    @override
    def do(self, original: Program) -> Program:
        decl = find_element(self.parent, original)
        if not isinstance(decl, elements.Function):
            raise BadEdit("Can only add interface elements to a function")
        new_id = next_id(decl)

        match self.params:
            case DeclParameterParams():
                decl.inputs.append(
                    elements.Parameter(
                        id=new_id, flType=self.flType, name=self.params.name
                    )
                )
            case DeclReturnParams():
                decl.outputs.append(
                    elements.Return(
                        id=new_id,
                        flType=self.flType,
                    )
                )
        self._added = new_id
        return original

    def undo(self, original: Program) -> Program:
        assert self._added is not None
        return RemoveItem(target=self.parent + [self._added]).do(original)

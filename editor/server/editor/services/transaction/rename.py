from typing import Literal, override

from pydantic import BaseModel

from editor.models import Program, QualifiedID, elements
from editor.models.edit_errors import BadEdit
from editor.models.elements import find_element
from editor.services.transaction.base import TransactionBase


class RenameDeclaration(BaseModel, TransactionBase):
    discriminator: Literal["rename_declaration"] = "rename_declaration"
    target: QualifiedID
    name: str

    @override
    def do(self, original: Program) -> Program:
        decl = find_element(self.target, original)
        if not isinstance(decl, elements.Declaration):
            raise BadEdit("Only a declaration may be renamed")
        decl.name, self.name = self.name, decl.name
        return original

    @override
    def undo(self, original: Program) -> Program:
        decl = find_element(self.target, original)
        assert isinstance(decl, elements.Declaration)
        decl.name, self.name = self.name, decl.name
        return original

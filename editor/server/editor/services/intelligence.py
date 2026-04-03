from editor.models.elements import Program
from editor.models.id import QualifiedID
from editor.models.lsp.diagnostic import Diagnostic
from editor.models.lsp.node_info import NodeInfo


class Intelligence:
    """A Service to provide intelligence for Fluir modules"""

    def add_module(self, module: Program, path: str) -> None:
        """Adds a module to generate intelligence data"""
        pass

    def remove_module(self, path: str) -> None:
        """Removes the given module"""
        pass

    # TODO: Allow incremental updates here (maybe using transaction types...)

    def get_node_info(self, path: str, element: QualifiedID) -> NodeInfo | None:
        """Returns the node info for the given element in the given path"""
        pass

    def get_diagnostics(self, path: str) -> list[Diagnostic] | None:
        """Returns all diagnostics for the given path"""
        pass

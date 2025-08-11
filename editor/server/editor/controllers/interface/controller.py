from abc import ABC, abstractmethod

from fastapi import FastAPI


class Controller(ABC):
    """A generic controller for editor requests"""

    @abstractmethod
    def register(self, app: FastAPI) -> None:
        """Registers the interface of this controller to the given interface"""

from pydantic.dataclasses import dataclass


@dataclass
class OpenRequest:
    path: str


@dataclass
class SaveRequest:
    path: str

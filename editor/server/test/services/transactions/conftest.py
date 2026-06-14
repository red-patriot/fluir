import copy
from pathlib import Path

import pytest

from editor.models import FlType, Program, elements
from editor.services.intelligence import (
    IntelligenceReadInterface,
    IntelligenceService,
)
from editor.services.module_editor import ModuleEditor


@pytest.fixture
def basic_program() -> Program:
    return Program(
        [
            elements.Function(
                name="foo",
                location=elements.Location(10, 10, 3, 100, 100),
                id=1,
            ),
            elements.Function(
                name="bar",
                location=elements.Location(210, 10, 2, 100, 100),
                id=2,
                nodes=[
                    elements.BinaryOperator(
                        id=1,
                        location=elements.Location(15, 2, 1, 5, 5),
                        op=elements.Operator.PLUS,
                    ),
                    elements.Constant(
                        id=2,
                        location=elements.Location(2, 2, 1, 5, 5),
                        value="3.0",
                        flType=FlType.F64,
                    ),
                    elements.Constant(
                        id=3,
                        location=elements.Location(2, 12, 1, 5, 5),
                        value="2.0",
                        flType=FlType.F64,
                    ),
                ],
                conduits=[
                    elements.Conduit(
                        id=5,
                        input=2,
                        children=[elements.Conduit.Output(target=1, index=1)],
                    )
                ],
            ),
            elements.Function(
                name="qux",
                location=elements.Location(210, 10, 2, 100, 110),
                id=3,
                nodes=[
                    elements.UnaryOperator(
                        id=2,
                        location=elements.Location(15, 2, 1, 5, 5),
                        op=elements.Operator.MINUS,
                    ),
                    elements.Constant(
                        id=3,
                        location=elements.Location(2, 12, 1, 5, 5),
                        value="2.0",
                        flType=FlType.F64,
                    ),
                ],
                conduits=[
                    elements.Conduit(
                        id=5,
                        input=3,
                        children=[elements.Conduit.Output(target=2, index=0)],
                    )
                ],
            ),
            elements.Function(
                name="xyzzy",
                location=elements.Location(410, 10, 2, 100, 100),
                id=4,
                inputs=[
                    elements.Parameter(name="a", id=1, flType=FlType.I32),
                    elements.Parameter(name="b", id=2, flType=FlType.I32),
                ],
                outputs=[
                    elements.Return(id=3, flType=FlType.I32),
                ],
            ),
        ]
    )


@pytest.fixture
def editor(basic_program: Program) -> ModuleEditor:
    editor = ModuleEditor()
    editor.open_module(copy.deepcopy(basic_program))
    return editor


@pytest.fixture
def intelligence(basic_program: Program) -> IntelligenceReadInterface:
    intelligence = IntelligenceService()
    intelligence.add_module(basic_program, Path("/fake/path.fl"))
    return intelligence


@pytest.fixture
def program_with_comments(basic_program: Program) -> Program:
    """basic_program with one top-level and one in-body comment preloaded."""
    program = copy.deepcopy(basic_program)
    program.annotations.append(
        elements.Comment(
            id=10,
            location=elements.Location(5, 5, 0, 20, 10),
            data="top-level comment",
        )
    )
    program.declarations[1].annotations.append(
        elements.Comment(
            id=20,
            location=elements.Location(3, 3, 0, 15, 8),
            data="in-body comment",
        )
    )
    return program


@pytest.fixture
def comments_editor(program_with_comments: Program) -> ModuleEditor:
    editor = ModuleEditor()
    editor.open_module(copy.deepcopy(program_with_comments))
    return editor


@pytest.fixture
def program_with_call(basic_program: Program) -> Program:
    """basic_program extended with a function containing a Call node."""
    program = copy.deepcopy(basic_program)
    program.declarations.append(
        elements.Function(
            name="call_host",
            location=elements.Location(610, 10, 2, 100, 100),
            id=100,
            nodes=[
                elements.Call(
                    id=1,
                    target="foo",
                    arguments=["x", "y"],
                ),
                elements.Constant(
                    id=2,
                    location=elements.Location(2, 2, 1, 5, 5),
                    value="1",
                    flType=FlType.I32,
                ),
            ],
            conduits=[
                elements.Conduit(
                    id=1,
                    input=2,
                    children=[elements.Conduit.Output(target=1, index=0)],
                )
            ],
        )
    )
    return program


@pytest.fixture
def call_editor(program_with_call: Program) -> ModuleEditor:
    editor = ModuleEditor()
    editor.open_module(copy.deepcopy(program_with_call))
    return editor


@pytest.fixture
def call_intelligence(program_with_call: Program) -> IntelligenceReadInterface:
    intelligence = IntelligenceService()
    intelligence.add_module(program_with_call, Path("/fake/path.fl"))
    return intelligence

from pathlib import Path

import pytest

from editor.models.elements import (
    BinaryOperator,
    Conduit,
    Constant,
    FlType,
    Function,
    Location,
    Operator,
    Program,
    UnaryOperator,
)
from editor.repository.fluir_file import XMLFileManager

_TEST_DATA = [
    (
        Program(
            [Function(name="foo", location=Location(10, 10, 3, 100, 100), id=1)]
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <function
            name="foo"
            id="1"
            x="10" y="10" z="3" w="100" h="100">
            <body>
            </body>
        </function>
    </fluir>
    """,
    ),
    (
        Program(
            [
                Function(
                    name="foo", location=Location(10, 10, 3, 100, 100), id=1
                ),
                Function(
                    name="baz", location=Location(330, 10, 3, 100, 100), id=7
                ),
                Function(
                    name="bar", location=Location(210, 10, 3, 50, 70), id=2
                ),
            ]
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <function
            name="foo"
            id="1"
            x="10" y="10" z="3" w="100" h="100">
            <body>
            </body>
        </function>
        <function
            name="baz"
            id="7"
            x="330" y="10" z="3" w="100" h="100">
            <body>
            </body>
        </function>
        <function
            name="bar"
            id="2"
            x="210" y="10" z="3" w="50" h="70">
            <body>
            </body>
        </function>
    </fluir>
    """,
    ),
    (
        Program(
            [
                Function(
                    name="foo",
                    location=Location(10, 10, 3, 100, 100),
                    id=1,
                    nodes=[
                        BinaryOperator(
                            id=1,
                            location=Location(15, 2, 1, 5, 5),
                            op=Operator.PLUS,
                        ),
                        Constant(
                            id=2,
                            location=Location(2, 2, 1, 5, 5),
                            value="3.0",
                            flType=FlType.F64,
                        ),
                        Constant(
                            id=3,
                            location=Location(2, 12, 1, 5, 5),
                            value="2.0",
                            flType=FlType.F64,
                        ),
                    ],
                    conduits=[
                        Conduit(
                            id=4, input=2, children=[Conduit.Output(target=1)]
                        ),
                        Conduit(
                            id=5, input=3, children=[Conduit.Output(target=1)]
                        ),
                    ],
                )
            ]
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <function
            name="foo"
            id="1"
            x="10" y="10" z="3" w="100" h="100">
            <body>
                <binary
                    id="1"
                    x="15" y="2" z="1" w="5" h="5"
                    operator="+" />
                <constant
                    id="2"
                    x="2" y="2" z="1" w="5" h="5">
                    <f64>3.0</f64>
                </constant>
                <constant
                    id="3"
                    x="2" y="12" z="1" w="5" h="5">
                    <f64>2.0</f64>
                </constant>
                <conduit id="4" input="2">
                    <output target="1"/>
                </conduit>
                <conduit id="5" input="3">
                    <output target="1"/>
                </conduit>
            </body>
        </function>
    </fluir>
    """,
    ),
    (
        Program(
            [
                Function(
                    name="main",
                    location=Location(10, 10, 3, 100, 100),
                    id=1,
                    nodes=[
                        UnaryOperator(
                            id=7,
                            location=Location(15, 2, 1, 5, 5),
                            op=Operator.MINUS,
                        ),
                        Constant(
                            id=3,
                            location=Location(2, 2, 1, 5, 5),
                            value="3.5",
                            flType=FlType.F64,
                        ),
                    ],
                    conduits=[
                        Conduit(
                            id=5,
                            input=3,
                            children=[Conduit.Output(target=7, index=0)],
                        ),
                    ],
                )
            ]
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <function
            name="main"
            id="1"
            x="10" y="10" z="3" w="100" h="100">
            <body>
                <unary
                    id="7"
                    x="15" y="2" z="1" w="5" h="5"
                    operator="-" />
                <constant
                    id="3"
                    x="2" y="2" z="1" w="5" h="5">
                    <f64>3.5</f64>
                </constant>
                <conduit id="5" input="3">
                    <output target="7" index="0"/>
                </conduit>
            </body>
        </function>
    </fluir>
    """,
    ),
    (
        Program(
            [
                Function(
                    name="someFuncName",
                    location=Location(10, 10, 3, 100, 100),
                    id=1,
                    nodes=[
                        BinaryOperator(
                            id=3,
                            location=Location(24, 39, 2, 5, 5),
                            op=Operator.SLASH,
                        ),
                        Constant(
                            id=2,
                            location=Location(6, 34, 0, 12, 5),
                            value="1.2345",
                            flType=FlType.F64,
                        ),
                        Constant(
                            id=1,
                            location=Location(6, 45, 0, 12, 5),
                            value="6.7890",
                            flType=FlType.F64,
                        ),
                        BinaryOperator(
                            id=5,
                            location=Location(54, 23, 2, 5, 5),
                            op=Operator.STAR,
                        ),
                        Constant(
                            id=4,
                            location=Location(29, 18, 0, 12, 5),
                            value="7.6543",
                            flType=FlType.F64,
                        ),
                        UnaryOperator(
                            id=6,
                            location=Location(35, 28, 0, 5, 5),
                            op=Operator.PLUS,
                        ),
                    ],
                    conduits=[
                        Conduit(
                            id=7,
                            input=2,
                            children=[
                                Conduit.Output(target=3, index=0),
                                Conduit.Output(target=6),
                            ],
                        ),
                        Conduit(
                            id=8,
                            input=1,
                            children=[
                                Conduit.Segment(
                                    x=84,
                                    y=19,
                                    children=[
                                        Conduit.Output(target=3, index=1)
                                    ],
                                )
                            ],
                        ),
                        Conduit(
                            id=9,
                            input=4,
                            children=[Conduit.Output(target=5, index=1)],
                        ),
                        Conduit(
                            id=10,
                            input=6,
                            children=[Conduit.Output(target=5, index=0)],
                        ),
                    ],
                )
            ]
        ),
        b"""<?xml version='1.0' encoding='UTF-8'?>
<fluir>
  <function name="someFuncName" id="1" x="10" y="10" z="3" w="100" h="100">
    <body>
      <binary id="3" x="24" y="39" z="2" w="5" h="5" operator="/" />
      <constant id="2" x="6" y="34" z="0" w="12" h="5">
        <f64>1.2345</f64>
      </constant>
      <constant id="1" x="6" y="45" z="0" w="12" h="5">
        <f64>6.7890</f64>
      </constant>
      <binary id="5" x="54" y="23" z="2" w="5" h="5" operator="*" />
      <constant id="4" x="29" y="18" z="0" w="12" h="5">
        <f64>7.6543</f64>
      </constant>
      <unary id="6" x="35" y="28" z="0" w="5" h="5" operator="+" />
      <conduit id="7" input="2">
        <output target="3" index="0"/>
        <output target="6"/>
      </conduit>
      <conduit id="8" input="1">
        <segment x="84" y="19">
          <output target="3" index="1"/>
        </segment>
      </conduit>
      <conduit id="9" input="4">
        <output target="5" index="1"/>
      </conduit>
      <conduit id="10" input="6">
        <output target="5" index="0"/>
      </conduit>
    </body>
  </function>
</fluir>""",
    ),
]


@pytest.mark.parametrize(
    "expected, data",
    _TEST_DATA,
    ids=[
        "single_empty_function",
        "multiple_empty_functions",
        "simple_binary_expr",
        "simple_unary_expr",
        "branching_conduits",
    ],
)
def test_repository_parses_string(expected: Program, data: bytes) -> None:
    uut = XMLFileManager()
    actual = uut.parseStr(data)

    assert expected == actual


def test_repository_opens_file(tmp_path: Path) -> None:
    filename = tmp_path / "test.fl"
    with open(filename, "w") as file:
        file.write("""<?xml version="1.0" encoding="UTF-8"?>
        <fluir>
            <function
                name="foo"
                id="1"
                x="10" y="10" z="3" w="100" h="100">
                <body>
                </body>
            </function>
        </fluir>
        """)

    expected = Program(
        [Function(name="foo", location=Location(10, 10, 3, 100, 100), id=1)]
    )

    uut = XMLFileManager()
    actual = uut.parseFile(filename)

    assert expected == actual

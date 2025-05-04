import pytest

from editor.models.elements import (
    BinaryOperator,
    Constant,
    Function,
    Location,
    Operator,
    Program,
)
from editor.repository.fluir_file import FileManager

_TEST_DATA = [
    (
        Program(
            {
                1: Function(
                    name="foo", location=Location(10, 10, 3, 100, 100), id=1
                )
            }
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir xmlns:fl="FLUIR::LANGUAGE::SOURCE">
        <fl:function
            name="foo"
            id="1"
            x="10" y="10" z="3" w="100" h="100">
            <body>
            </body>
        </fl:function>
    </fluir>
    """,
    ),
    (
        Program(
            {
                1: Function(
                    name="foo", location=Location(10, 10, 3, 100, 100), id=1
                ),
                2: Function(
                    name="bar", location=Location(210, 10, 3, 50, 70), id=2
                ),
                7: Function(
                    name="baz", location=Location(330, 10, 3, 100, 100), id=7
                ),
            }
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir xmlns:fl="FLUIR::LANGUAGE::SOURCE">
        <fl:function
            name="foo"
            id="1"
            x="10" y="10" z="3" w="100" h="100">
            <body>
            </body>
        </fl:function>
        <fl:function
            name="baz"
            id="7"
            x="330" y="10" z="3" w="100" h="100">
            <body>
            </body>
        </fl:function>
        <fl:function
            name="bar"
            id="2"
            x="210" y="10" z="3" w="50" h="70">
            <body>
            </body>
        </fl:function>
    </fluir>
    """,
    ),
    (
        Program(
            {
                1: Function(
                    name="foo",
                    location=Location(10, 10, 3, 100, 100),
                    id=1,
                    body={
                        1: BinaryOperator(
                            id=1,
                            location=Location(15, 2, 1, 5, 5),
                            op=Operator.PLUS,
                            lhs=2,
                            rhs=3,
                        ),
                        2: Constant(
                            id=2, location=Location(2, 2, 1, 5, 5), value=3.0
                        ),
                        3: Constant(
                            id=3, location=Location(2, 12, 1, 5, 5), value=2.0
                        ),
                    },
                )
            }
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir xmlns:fl="FLUIR::LANGUAGE::SOURCE">
        <fl:function
            name="foo"
            id="1"
            x="10" y="10" z="3" w="100" h="100">
            <body>
                <fl:binary
                    id="1"
                    x="15" y="2" z="1" w="5" h="5"
                    lhs="2"
                    rhs="3"
                    operator="+" />
                <fl:constant
                    id="2"
                    x="2" y="2" z="1" w="5" h="5">
                    <fl:float>3.0</fl:float>
                </fl:constant>
                <fl:constant
                    id="3"
                    x="2" y="12" z="1" w="5" h="5">
                    <fl:float>2.0</fl:float>
                </fl:constant>
            </body>
        </fl:function>
    </fluir>
    """,
    ),
]


@pytest.mark.parametrize(
    "expected, data",
    _TEST_DATA,
    ids=[
        "single_empty_function",
        "multiple_empty_functions",
        "simple_binary_expr",
    ],
)
def test_repository_parses_string(expected: Program, data: bytes) -> None:
    uut = FileManager()
    actual = uut.parseStr(data)

    assert expected == actual

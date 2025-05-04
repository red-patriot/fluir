from editor.models.elements import Function, Location, Program
from editor.repository.fluir_file import FileManager


def test_repository_parses_function_without_body() -> None:
    expected = Program(
        {1: Function(name="foo", location=Location(10, 10, 3, 100, 100), id=1)}
    )
    data = b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir xmlns:fl="FLUIR::LANGUAGE::SOURCE">
        <fl:function
            name="foo"
            id="1"
            x="10" y="10" z="3" w="100" h="100">
            <body>
            </body>
        </fl:function>
    </fluir>
    """

    uut = FileManager()
    actual = uut.parseStr(data)

    assert expected == actual


def test_repository_parses_multiple_functions_without_body() -> None:
    expected = Program(
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
    )
    data = b"""<?xml version="1.0" encoding="UTF-8"?>
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
    """

    uut = FileManager()
    actual = uut.parseStr(data)

    assert expected == actual

import importlib.util
from pathlib import Path

import pytest

from editor.models import Function, Header, Location, Program, Version
from editor.repository.fluir_file import XMLFileManager

_PROGRAMS_DIR = Path(__file__).parent / "programs" / "read"


def _load_cases() -> list[tuple[str, Program, bytes]]:
    cases: list[tuple[str, Program, bytes]] = []
    for fl in sorted(_PROGRAMS_DIR.glob("*.fl")):
        py = fl.with_suffix(".py")
        spec = importlib.util.spec_from_file_location(fl.stem, py)
        assert spec is not None and spec.loader is not None
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        cases.append((fl.stem, module.program, fl.read_bytes()))
    return cases


_CASES = _load_cases()


@pytest.mark.parametrize(
    "expected, data",
    [(program, data) for _, program, data in _CASES],
    ids=[name for name, _, _ in _CASES],
)
def test_repository_parses_string(expected: Program, data: bytes) -> None:
    actual = XMLFileManager().parseStr(data)
    assert expected == actual


def test_repository_opens_file(tmp_path: Path) -> None:
    filename = tmp_path / "test.fl"
    with open(filename, "w") as file:
        file.write("""<?xml version="1.0" encoding="UTF-8"?>
        <fluir>
            <header>
                <version>
                    <major>0</major>
                    <minor>1</minor>
                    <patch>3</patch>
                </version>
            </header>
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
        [Function(name="foo", location=Location(10, 10, 3, 100, 100), id=1)],
        Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
    )

    uut = XMLFileManager()
    actual = uut.parseFile(filename)

    assert expected == actual

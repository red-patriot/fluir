import importlib.util
from pathlib import Path

import pytest

from editor.models import Program
from editor.repository.fluir_file import XMLFileManager

_PROGRAMS_DIR = Path(__file__).parent / "programs" / "write"


def _load_cases() -> list[tuple[str, Program, str]]:
    cases: list[tuple[str, Program, str]] = []
    for fl in sorted(_PROGRAMS_DIR.glob("*.fl")):
        py = fl.with_suffix(".py")
        spec = importlib.util.spec_from_file_location(fl.stem, py)
        assert spec is not None and spec.loader is not None
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        cases.append((fl.stem, module.program, fl.read_text()))
    return cases


_CASES = _load_cases()


@pytest.mark.parametrize(
    "data, expected",
    [(program, expected) for _, program, expected in _CASES],
    ids=[name for name, _, _ in _CASES],
)
def test_repository_writes_file(
    tmp_path: Path, data: Program, expected: str
) -> None:
    path = tmp_path / "prog.fl"
    XMLFileManager().writeFile(data, path)
    assert path.read_text() == expected

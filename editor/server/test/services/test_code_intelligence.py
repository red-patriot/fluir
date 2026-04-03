from pathlib import Path

import pytest

from test_utils.test_files import file_path_name, get_test_programs


@pytest.mark.parametrize(
    "program",
    get_test_programs("type_check", extension=".fl"),
    ids=file_path_name,
)
def test_type_check(program: Path) -> None:
    assert False, "TODO: implement type check intelligence tests"


@pytest.mark.parametrize(
    "program",
    get_test_programs("type_errors", extension=".fl"),
    ids=file_path_name,
)
def test_type_errors(program: Path) -> None:
    assert False, "TODO: implement type errors intelligence tests"

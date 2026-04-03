from pathlib import Path

_TEST_PROGRAMS_DIR: Path = Path(__file__).resolve().parents[3] / "test_programs"
_INTERNAL_PROGRAMS_DIR: Path = Path(__file__).parent / "test_programs"


def get_test_programs(
    relative: str | Path, extension: str = ".json"
) -> list[Path]:
    """Gets all test programs in the given directory relative to test_programs/."""
    absolute_parent = _TEST_PROGRAMS_DIR / relative
    return [
        entry
        for entry in absolute_parent.iterdir()
        if entry.is_file() and entry.suffix == extension
    ]


def get_test_program(relative: str | Path) -> Path:
    """Gets the absolute path given a path relative to test_programs/."""
    absolute = _TEST_PROGRAMS_DIR / relative
    if not absolute.is_file():
        raise RuntimeError(f"Expected a filepath, got {relative}")
    return absolute


def get_relative_path(program_file: Path) -> Path:
    """Gets the relative path of a file inside the test_programs/ directory."""
    return program_file.relative_to(_TEST_PROGRAMS_DIR)


def read_contents(file: Path) -> str:
    """Reads the contents of a file into a string."""
    return file.read_text()


def file_path_name(path: Path) -> str:
    """Gets the name of a test file (stem without final extension)."""
    return path.stem


def get_golden_file_path(relative: str | Path) -> Path:
    """Gets the absolute path given a path relative to test_programs/."""
    absolute = _INTERNAL_PROGRAMS_DIR / relative
    if not absolute.is_file():
        raise RuntimeError(f"Expected a filepath, got {relative}")
    return absolute

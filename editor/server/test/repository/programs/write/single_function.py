from editor.models import (
    Function,
    Header,
    Location,
    Program,
    Version,
)

program: Program = Program(
    [Function(name="foo", location=Location(10, 10, 3, 100, 100), id=1)],
    Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
)

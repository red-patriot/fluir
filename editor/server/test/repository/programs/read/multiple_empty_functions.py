from editor.models import Function, Header, Location, Program, Version

program: Program = Program(
    [
        Function(name="foo", location=Location(10, 10, 3, 100, 100), id=1),
        Function(name="baz", location=Location(330, 10, 3, 100, 100), id=7),
        Function(name="bar", location=Location(210, 10, 3, 50, 70), id=2),
    ],
    Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
)

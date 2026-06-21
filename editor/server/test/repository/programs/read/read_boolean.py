from editor.models import (
    Constant,
    FlType,
    Function,
    Header,
    Location,
    Program,
    Version,
)

program: Program = Program(
    [
        Function(
            name="main",
            location=Location(10, 10, 3, 100, 100),
            id=1,
            nodes=[
                Constant(
                    id=1,
                    location=Location(2, 20, 1, 5, 5),
                    value="false",
                    flType=FlType.BOOL,
                ),
                Constant(
                    id=2,
                    location=Location(12, 21, 1, 5, 5),
                    value="true",
                    flType=FlType.BOOL,
                ),
            ],
        )
    ],
    Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
)

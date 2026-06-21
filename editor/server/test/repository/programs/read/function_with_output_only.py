from editor.models import (
    FlType,
    Function,
    Header,
    Location,
    Program,
    Return,
    Version,
)

program: Program = Program(
    [
        Function(
            name="getVal",
            location=Location(10, 10, 3, 100, 100),
            id=1,
            outputs=[
                Return(id=4, flType=FlType.F64),
            ],
        )
    ],
    Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
)

from editor.models import (
    FlType,
    Function,
    Header,
    Location,
    Parameter,
    Program,
    Return,
    Version,
)

program: Program = Program(
    [
        Function(
            name="transform",
            location=Location(10, 10, 3, 100, 100),
            id=1,
            inputs=[
                Parameter(name="x", id=2, flType=FlType.F64),
            ],
            outputs=[
                Return(id=3, flType=FlType.F64),
                Return(id=4, flType=FlType.I32),
            ],
        )
    ],
    Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
)

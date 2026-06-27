from editor.models import (
    FlType,
    Function,
    Header,
    Location,
    Parameter,
    Program,
    Version,
)

program: Program = Program(
    [
        Function(
            name="add",
            location=Location(10, 10, 3, 100, 100),
            id=1,
            inputs=[
                Parameter(name="a", id=2, flType=FlType.I32),
                Parameter(name="b", id=3, flType=FlType.I32),
            ],
        )
    ],
    Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
)

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
                    value="5",
                    flType=FlType.U8,
                ),
                Constant(
                    id=2,
                    location=Location(12, 21, 1, 5, 5),
                    value="318",
                    flType=FlType.U16,
                ),
                Constant(
                    id=3,
                    location=Location(22, 22, 1, 5, 5),
                    value="324",
                    flType=FlType.U32,
                ),
                Constant(
                    id=4,
                    location=Location(32, 23, 1, 5, 5),
                    value="122",
                    flType=FlType.U64,
                ),
            ],
        )
    ],
    Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
)

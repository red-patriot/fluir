from editor.models import (
    Call,
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
            location=Location(0, 0, 0, 100, 100),
            id=1,
            nodes=[
                Call(
                    id=2,
                    location=Location(5, 5, 0, 12, 12),
                    target="doStuff",
                    arguments=[],
                    returns=False,
                ),
            ],
        )
    ],
    Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
)

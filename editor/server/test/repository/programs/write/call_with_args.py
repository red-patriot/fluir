from editor.models import (
    Call,
    Conduit,
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
            location=Location(0, 0, 0, 100, 100),
            id=1,
            nodes=[
                Constant(
                    id=1,
                    location=Location(0, 0, 0, 5, 5),
                    value="10",
                    flType=FlType.I32,
                ),
                Constant(
                    id=2,
                    location=Location(0, 20, 0, 5, 5),
                    value="20",
                    flType=FlType.I32,
                ),
                Call(
                    id=3,
                    location=Location(30, 10, 0, 12, 12),
                    target="add",
                    arguments=["a", "b"],
                    returns=True,
                ),
            ],
            conduits=[
                Conduit(
                    id=4,
                    input=1,
                    children=[Conduit.Output(target=3, index=0)],
                ),
                Conduit(
                    id=5,
                    input=2,
                    children=[Conduit.Output(target=3, index=1)],
                ),
            ],
        )
    ],
    Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
)

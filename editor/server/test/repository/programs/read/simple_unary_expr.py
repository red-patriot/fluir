from editor.models import (
    Conduit,
    Constant,
    FlType,
    Function,
    Header,
    Location,
    Operator,
    Program,
    UnaryOperator,
    Version,
)

program: Program = Program(
    [
        Function(
            name="main",
            location=Location(10, 10, 3, 100, 100),
            id=1,
            nodes=[
                UnaryOperator(
                    id=7,
                    location=Location(15, 2, 1, 5, 5),
                    op=Operator.MINUS,
                ),
                Constant(
                    id=3,
                    location=Location(2, 2, 1, 5, 5),
                    value="3.5",
                    flType=FlType.F64,
                ),
            ],
            conduits=[
                Conduit(
                    id=5,
                    input=3,
                    children=[Conduit.Output(target=7, index=0)],
                ),
            ],
        )
    ],
    Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
)

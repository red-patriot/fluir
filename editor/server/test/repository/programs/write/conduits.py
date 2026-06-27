from editor.models import (
    BinaryOperator,
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
            name="someFuncName",
            location=Location(10, 10, 3, 100, 100),
            id=1,
            nodes=[
                BinaryOperator(
                    id=3,
                    location=Location(24, 39, 2, 5, 5),
                    op=Operator.SLASH,
                ),
                Constant(
                    id=2,
                    location=Location(6, 34, 0, 12, 5),
                    value="1.2345",
                    flType=FlType.F64,
                ),
                Constant(
                    id=1,
                    location=Location(6, 45, 0, 12, 5),
                    value="6.7890",
                    flType=FlType.F64,
                ),
                BinaryOperator(
                    id=5,
                    location=Location(54, 23, 2, 5, 5),
                    op=Operator.STAR,
                ),
                Constant(
                    id=4,
                    location=Location(29, 18, 0, 12, 5),
                    value="7.6543",
                    flType=FlType.F64,
                ),
                UnaryOperator(
                    id=6,
                    location=Location(35, 28, 0, 5, 5),
                    op=Operator.PLUS,
                ),
            ],
            conduits=[
                Conduit(
                    id=7,
                    input=2,
                    children=[
                        Conduit.Output(target=3, index=0),
                        Conduit.Output(target=6),
                    ],
                ),
                Conduit(
                    id=8,
                    input=1,
                    children=[
                        Conduit.Segment(
                            x=84,
                            y=19,
                            children=[Conduit.Output(target=3, index=1)],
                        )
                    ],
                ),
                Conduit(
                    id=9,
                    input=4,
                    children=[Conduit.Output(target=5, index=1)],
                ),
                Conduit(
                    id=10,
                    input=6,
                    children=[Conduit.Output(target=5, index=0)],
                ),
            ],
        )
    ],
    Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
)

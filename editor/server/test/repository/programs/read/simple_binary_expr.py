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
    Version,
)

program: Program = Program(
    [
        Function(
            name="foo",
            location=Location(10, 10, 3, 100, 100),
            id=1,
            nodes=[
                BinaryOperator(
                    id=1,
                    location=Location(15, 2, 1, 5, 5),
                    op=Operator.PLUS,
                ),
                Constant(
                    id=2,
                    location=Location(2, 2, 1, 5, 5),
                    value="3.0",
                    flType=FlType.F64,
                ),
                Constant(
                    id=3,
                    location=Location(2, 12, 1, 5, 5),
                    value="2.0",
                    flType=FlType.F64,
                ),
            ],
            conduits=[
                Conduit(id=4, input=2, children=[Conduit.Output(target=1)]),
                Conduit(id=5, input=3, children=[Conduit.Output(target=1)]),
            ],
        )
    ],
    Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
)

from editor.models import (
    Constant,
    FlType,
    Function,
    Header,
    Location,
    Program,
    Version,
)
from editor.models.elements import Comment

program: Program = Program(
    declarations=[
        Function(
            name="main",
            location=Location(10, 10, 3, 100, 100),
            id=1,
            nodes=[
                Constant(
                    id=2,
                    location=Location(2, 2, 1, 5, 5),
                    value="3",
                    flType=FlType.I32,
                ),
            ],
            annotations=[
                Comment(
                    id=1,
                    location=Location(10, 10, 4, 25, 25),
                    data="Hello there! This is a simple comment!",
                ),
            ],
        )
    ],
    header=Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
    annotations=[
        Comment(
            id=2,
            location=Location(1050, 10, 4, 25, 25),
            data="Comments are also allowed at the top level",
        ),
    ],
)

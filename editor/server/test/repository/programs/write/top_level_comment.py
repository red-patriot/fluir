from editor.models import (
    Header,
    Location,
    Program,
    Version,
)
from editor.models.elements import Comment

program: Program = Program(
    declarations=[],
    header=Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
    annotations=[
        Comment(
            id=1,
            location=Location(10, 10, 4, 25, 25),
            data="hello",
        ),
    ],
)

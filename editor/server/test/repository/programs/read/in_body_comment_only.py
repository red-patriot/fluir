from editor.models import Function, Header, Location, Program, Version
from editor.models.elements import Comment

program: Program = Program(
    [
        Function(
            name="main",
            location=Location(0, 0, 0, 100, 100),
            id=1,
            annotations=[
                Comment(
                    id=2,
                    location=Location(5, 5, 4, 25, 25),
                    data="note",
                ),
            ],
        )
    ],
    Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
)

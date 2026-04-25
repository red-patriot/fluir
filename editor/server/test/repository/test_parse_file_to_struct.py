from pathlib import Path

import pytest

from editor.models import (
    BinaryOperator,
    Call,
    Conduit,
    Constant,
    FlType,
    Function,
    Header,
    Location,
    Operator,
    Parameter,
    Program,
    Return,
    UnaryOperator,
    Version,
)
from editor.models.elements import Comment
from editor.repository.fluir_file import XMLFileManager

_TEST_DATA = [
    (
        Program(
            [
                Function(
                    name="foo", location=Location(10, 10, 3, 100, 100), id=1
                )
            ],
            Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <header>
            <version>
                <major>0</major>
                <minor>1</minor>
                <patch>3</patch>
            </version>
        </header>
        <function
            name="foo"
            id="1"
            x="10" y="10" z="3" w="100" h="100">
            <body>
            </body>
        </function>
    </fluir>
    """,
    ),
    (
        Program(
            [
                Function(
                    name="foo", location=Location(10, 10, 3, 100, 100), id=1
                ),
                Function(
                    name="baz", location=Location(330, 10, 3, 100, 100), id=7
                ),
                Function(
                    name="bar", location=Location(210, 10, 3, 50, 70), id=2
                ),
            ],
            Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <header>
            <version>
                <major>0</major>
                <minor>1</minor>
                <patch>3</patch>
            </version>
        </header>
        <function
            name="foo"
            id="1"
            x="10" y="10" z="3" w="100" h="100">
            <body>
            </body>
        </function>
        <function
            name="baz"
            id="7"
            x="330" y="10" z="3" w="100" h="100">
            <body>
            </body>
        </function>
        <function
            name="bar"
            id="2"
            x="210" y="10" z="3" w="50" h="70">
            <body>
            </body>
        </function>
    </fluir>
    """,
    ),
    (
        Program(
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
                        Conduit(
                            id=4, input=2, children=[Conduit.Output(target=1)]
                        ),
                        Conduit(
                            id=5, input=3, children=[Conduit.Output(target=1)]
                        ),
                    ],
                )
            ],
            Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <header>
            <version>
                <major>0</major>
                <minor>1</minor>
                <patch>3</patch>
            </version>
        </header>
        <function
            name="foo"
            id="1"
            x="10" y="10" z="3" w="100" h="100">
            <body>
                <binary
                    id="1"
                    x="15" y="2" z="1" w="5" h="5"
                    operator="+" />
                <constant
                    id="2"
                    x="2" y="2" z="1" w="5" h="5">
                    <f64>3.0</f64>
                </constant>
                <constant
                    id="3"
                    x="2" y="12" z="1" w="5" h="5">
                    <f64>2.0</f64>
                </constant>
                <conduit id="4" input="2">
                    <output target="1"/>
                </conduit>
                <conduit id="5" input="3">
                    <output target="1"/>
                </conduit>
            </body>
        </function>
    </fluir>
    """,
    ),
    (
        Program(
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
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <header>
            <version>
                <major>0</major>
                <minor>1</minor>
                <patch>3</patch>
            </version>
        </header>
        <function
            name="main"
            id="1"
            x="10" y="10" z="3" w="100" h="100">
            <body>
                <unary
                    id="7"
                    x="15" y="2" z="1" w="5" h="5"
                    operator="-" />
                <constant
                    id="3"
                    x="2" y="2" z="1" w="5" h="5">
                    <f64>3.5</f64>
                </constant>
                <conduit id="5" input="3">
                    <output target="7" index="0"/>
                </conduit>
            </body>
        </function>
    </fluir>
    """,
    ),
    (
        Program(
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
                                    children=[
                                        Conduit.Output(target=3, index=1)
                                    ],
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
        ),
        b"""<?xml version='1.0' encoding='UTF-8'?>
<fluir>
  <header>
    <version>
      <major>0</major>
      <minor>1</minor>
      <patch>3</patch>
    </version>
  </header>
  <function name="someFuncName" id="1" x="10" y="10" z="3" w="100" h="100">
    <body>
      <binary id="3" x="24" y="39" z="2" w="5" h="5" operator="/" />
      <constant id="2" x="6" y="34" z="0" w="12" h="5">
        <f64>1.2345</f64>
      </constant>
      <constant id="1" x="6" y="45" z="0" w="12" h="5">
        <f64>6.7890</f64>
      </constant>
      <binary id="5" x="54" y="23" z="2" w="5" h="5" operator="*" />
      <constant id="4" x="29" y="18" z="0" w="12" h="5">
        <f64>7.6543</f64>
      </constant>
      <unary id="6" x="35" y="28" z="0" w="5" h="5" operator="+" />
      <conduit id="7" input="2">
        <output target="3" index="0"/>
        <output target="6"/>
      </conduit>
      <conduit id="8" input="1">
        <segment x="84" y="19">
          <output target="3" index="1"/>
        </segment>
      </conduit>
      <conduit id="9" input="4">
        <output target="5" index="1"/>
      </conduit>
      <conduit id="10" input="6">
        <output target="5" index="0"/>
      </conduit>
    </body>
  </function>
</fluir>""",
    ),
    (
        Program(
            [
                Function(
                    name="main",
                    location=Location(10, 10, 3, 100, 100),
                    id=1,
                    nodes=[
                        Constant(
                            id=1,
                            location=Location(2, 20, 1, 5, 5),
                            value="-5",
                            flType=FlType.I8,
                        ),
                        Constant(
                            id=2,
                            location=Location(12, 21, 1, 5, 5),
                            value="318",
                            flType=FlType.I16,
                        ),
                        Constant(
                            id=3,
                            location=Location(22, 22, 1, 5, 5),
                            value="324",
                            flType=FlType.I32,
                        ),
                        Constant(
                            id=4,
                            location=Location(32, 23, 1, 5, 5),
                            value="-12",
                            flType=FlType.I64,
                        ),
                    ],
                )
            ],
            Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <header>
            <version>
                <major>0</major>
                <minor>1</minor>
                <patch>3</patch>
            </version>
        </header>
        <function
            name="main"
            id="1"
            x="10" y="10" z="3" w="100" h="100">
            <body>
                <constant
                    id="1"
                    x="2" y="20" z="1" w="5" h="5">
                    <i8>-5</i8>
                </constant>
                <constant
                    id="2"
                    x="12" y="21" z="1" w="5" h="5">
                    <i16>318</i16>
                </constant>
                <constant
                    id="3"
                    x="22" y="22" z="1" w="5" h="5">
                    <i32>324</i32>
                </constant>
                <constant
                    id="4"
                    x="32" y="23" z="1" w="5" h="5">
                    <i64>-12</i64>
                </constant>
            </body>
        </function>
    </fluir>
    """,
    ),
    (
        Program(
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
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <header>
            <version>
                <major>0</major>
                <minor>1</minor>
                <patch>3</patch>
            </version>
        </header>
        <function
            name="main"
            id="1"
            x="10" y="10" z="3" w="100" h="100">
            <body>
                <constant
                    id="1"
                    x="2" y="20" z="1" w="5" h="5">
                    <u8>5</u8>
                </constant>
                <constant
                    id="2"
                    x="12" y="21" z="1" w="5" h="5">
                    <u16>318</u16>
                </constant>
                <constant
                    id="3"
                    x="22" y="22" z="1" w="5" h="5">
                    <u32>324</u32>
                </constant>
                <constant
                    id="4"
                    x="32" y="23" z="1" w="5" h="5">
                    <u64>122</u64>
                </constant>
            </body>
        </function>
    </fluir>
    """,
    ),
    (
        Program(
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
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <header>
            <version>
                <major>0</major>
                <minor>1</minor>
                <patch>3</patch>
            </version>
        </header>
        <function name="add" id="1" x="10" y="10" z="3" w="100" h="100">
            <input>
                <param name="a" id="2" type="I32"/>
                <param name="b" id="3" type="I32"/>
            </input>
            <body/>
        </function>
    </fluir>
    """,
    ),
    (
        Program(
            [
                Function(
                    name="getVal",
                    location=Location(10, 10, 3, 100, 100),
                    id=1,
                    outputs=[
                        Return(id=4, flType=FlType.F64),
                    ],
                )
            ],
            Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <header>
            <version>
                <major>0</major>
                <minor>1</minor>
                <patch>3</patch>
            </version>
        </header>
        <function name="getVal" id="1" x="10" y="10" z="3" w="100" h="100">
            <output>
                <return id="4" type="F64"/>
            </output>
            <body/>
        </function>
    </fluir>
    """,
    ),
    (
        Program(
            [
                Function(
                    name="transform",
                    location=Location(10, 10, 3, 100, 100),
                    id=1,
                    inputs=[
                        Parameter(name="x", id=2, flType=FlType.F64),
                    ],
                    outputs=[
                        Return(id=3, flType=FlType.F64),
                        Return(id=4, flType=FlType.I32),
                    ],
                )
            ],
            Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <header>
            <version>
                <major>0</major>
                <minor>1</minor>
                <patch>3</patch>
            </version>
        </header>
        <function name="transform" id="1" x="10" y="10" z="3" w="100" h="100">
            <input>
                <param name="x" id="2" type="F64"/>
            </input>
            <output>
                <return id="3" type="F64"/>
                <return id="4" type="I32"/>
            </output>
            <body/>
        </function>
    </fluir>
    """,
    ),
    (
        Program(
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
                            children=[Conduit.Output(target=3, index=1)],
                        ),
                        Conduit(
                            id=5,
                            input=2,
                            children=[Conduit.Output(target=3, index=2)],
                        ),
                    ],
                )
            ],
            Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <header>
            <version>
                <major>0</major>
                <minor>1</minor>
                <patch>3</patch>
            </version>
        </header>
        <function name="main" id="1" x="0" y="0" z="0" w="100" h="100">
            <body>
                <constant id="1" x="0" y="0" z="0" w="5" h="5">
                    <i32>10</i32>
                </constant>
                <constant id="2" x="0" y="20" z="0" w="5" h="5">
                    <i32>20</i32>
                </constant>
                <call target="add" id="3" x="30" y="10" z="0" w="12" h="12">
                    <return index="0"/>
                    <arg name="a" index="1"/>
                    <arg name="b" index="2"/>
                </call>
                <conduit id="4" input="1"><output target="3" index="1"/></conduit>
                <conduit id="5" input="2"><output target="3" index="2"/></conduit>
            </body>
        </function>
    </fluir>
    """,
    ),
    (
        Program(
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
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <header>
            <version>
                <major>0</major>
                <minor>1</minor>
                <patch>3</patch>
            </version>
        </header>
        <function name="main" id="1" x="0" y="0" z="0" w="100" h="100">
            <body>
                <call target="doStuff" id="2" x="5" y="5" z="0" w="12" h="12"/>
            </body>
        </function>
    </fluir>
    """,
    ),
    (
        Program(
            [
                Function(
                    name="main",
                    location=Location(0, 0, 0, 100, 100),
                    id=1,
                    nodes=[
                        Call(
                            id=2,
                            location=Location(5, 5, 0, 12, 12),
                            target="reorder",
                            arguments=["first", "second", "third"],
                            returns=False,
                        ),
                    ],
                )
            ],
            Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <header>
            <version>
                <major>0</major>
                <minor>1</minor>
                <patch>3</patch>
            </version>
        </header>
        <function name="main" id="1" x="0" y="0" z="0" w="100" h="100">
            <body>
                <call target="reorder" id="2" x="5" y="5" z="0" w="12" h="12">
                    <arg name="third" index="2"/>
                    <arg name="first" index="0"/>
                    <arg name="second" index="1"/>
                </call>
            </body>
        </function>
    </fluir>
    """,
    ),
    (
        Program(
            declarations=[],
            header=Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
            annotations=[
                Comment(
                    id=1,
                    location=Location(10, 10, 4, 25, 25),
                    data="hello",
                ),
            ],
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <header>
            <version>
                <major>0</major>
                <minor>1</minor>
                <patch>3</patch>
            </version>
        </header>
        <comment id="1" x="10" y="10" z="4" w="25" h="25">hello</comment>
    </fluir>
    """,
    ),
    (
        Program(
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
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <header>
            <version>
                <major>0</major>
                <minor>1</minor>
                <patch>3</patch>
            </version>
        </header>
        <function name="main" id="1" x="0" y="0" z="0" w="100" h="100">
            <body>
                <comment id="2" x="5" y="5" z="4" w="25" h="25">note</comment>
            </body>
        </function>
    </fluir>
    """,
    ),
    (
        Program(
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
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <header>
            <version>
                <major>0</major>
                <minor>1</minor>
                <patch>3</patch>
            </version>
        </header>
        <function name="main" id="1" x="10" y="10" z="3" w="100" h="100">
            <body>
                <constant id="2" x="2" y="2" z="1" w="5" h="5">
                    <i32>3</i32>
                </constant>
                <comment id="1" x="10" y="10" z="4" w="25" h="25">Hello there! This is a simple comment!</comment>
            </body>
        </function>
        <comment id="2" x="1050" y="10" z="4" w="25" h="25">Comments are also allowed at the top level</comment>
    </fluir>
    """,
    ),
    (
        Program(
            declarations=[],
            header=Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
            annotations=[
                Comment(
                    id=1,
                    location=Location(10, 10, 4, 25, 25),
                    data="",
                ),
            ],
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <header>
            <version>
                <major>0</major>
                <minor>1</minor>
                <patch>3</patch>
            </version>
        </header>
        <comment id="1" x="10" y="10" z="4" w="25" h="25"></comment>
    </fluir>
    """,
    ),
    (
        Program(
            declarations=[],
            header=Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
            annotations=[
                Comment(
                    id=1,
                    location=Location(10, 10, 4, 25, 25),
                    data="Hello there! This is a simple comment!",
                ),
            ],
        ),
        b"""<?xml version="1.0" encoding="UTF-8"?>
    <fluir>
        <header>
            <version>
                <major>0</major>
                <minor>1</minor>
                <patch>3</patch>
            </version>
        </header>
        <comment id="1" x="10" y="10" z="4" w="25" h="25">Hello there! This is a simple comment!</comment>
    </fluir>
    """,
    ),
]


@pytest.mark.parametrize(
    "expected, data",
    _TEST_DATA,
    ids=[
        "single_empty_function",
        "multiple_empty_functions",
        "simple_binary_expr",
        "simple_unary_expr",
        "branching_conduits",
        "int_constants",
        "uint_constants",
        "function_with_input_only",
        "function_with_output_only",
        "function_with_input_and_output",
        "function_call",
        "function_call_no_args_no_returns",
        "function_call_args_out_of_order",
        "top_level_comment_only",
        "in_body_comment_only",
        "mixed_comments",
        "empty_comment_data",
        "comment_with_whitespace_and_punctuation",
    ],
)
def test_repository_parses_string(expected: Program, data: bytes) -> None:
    uut = XMLFileManager()
    actual = uut.parseStr(data)

    assert expected == actual


def test_repository_opens_file(tmp_path: Path) -> None:
    filename = tmp_path / "test.fl"
    with open(filename, "w") as file:
        file.write("""<?xml version="1.0" encoding="UTF-8"?>
        <fluir>
            <header>
                <version>
                    <major>0</major>
                    <minor>1</minor>
                    <patch>3</patch>
                </version>
            </header>
            <function
                name="foo"
                id="1"
                x="10" y="10" z="3" w="100" h="100">
                <body>
                </body>
            </function>
        </fluir>
        """)

    expected = Program(
        [Function(name="foo", location=Location(10, 10, 3, 100, 100), id=1)],
        Header(version=Version(MAJOR=0, MINOR=1, PATCH=3)),
    )

    uut = XMLFileManager()
    actual = uut.parseFile(filename)

    assert expected == actual

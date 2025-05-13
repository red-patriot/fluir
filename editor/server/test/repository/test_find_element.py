import pytest

from editor.models import INVALID_ID, FlType, IDType, Program, elements


@pytest.fixture
def basic_program() -> Program:
    return Program(
        [
            elements.Function(
                name="foo",
                location=elements.Location(10, 10, 3, 100, 100),
                id=1,
            ),
            elements.Function(
                name="bar",
                location=elements.Location(210, 10, 2, 150, 70),
                id=2,
                body=[
                    elements.BinaryOperator(
                        id=1,
                        location=elements.Location(15, 2, 1, 5, 5),
                        op=elements.Operator.PLUS,
                        lhs=2,
                        rhs=3,
                    ),
                    elements.Constant(
                        id=2,
                        location=elements.Location(2, 2, 1, 5, 5),
                        value="3.0",
                        flType=FlType.FLOATING_POINT,
                    ),
                    elements.Constant(
                        id=3,
                        location=elements.Location(2, 12, 1, 5, 5),
                        flType=FlType.FLOATING_POINT,
                        value="2.0",
                    ),
                ],
            ),
        ]
    )


def test_find_element_finds_function(basic_program: Program) -> None:
    expected_name = "foo"
    expected_id = 1
    target = [1]

    actual = elements.find_element(target, basic_program)

    assert isinstance(actual, elements.Function)
    assert expected_name == actual.name
    assert expected_id == actual.id


def test_find_element_raises_if_not_found(basic_program: Program) -> None:
    target = [1, 3, 4]

    with pytest.raises(
        elements.IdentifierError, match="An element with ID 1:3:4 was not found"
    ):
        elements.find_element(target, basic_program)


def test_find_element_raises_if_id_is_empty(basic_program: Program) -> None:
    target: list[IDType] = []

    with pytest.raises(elements.IdentifierError, match="ID is invalid"):
        elements.find_element(target, basic_program)


def test_find_element_raises_if_id_is_invalid(basic_program: Program) -> None:
    target: list[IDType] = [1, INVALID_ID]

    with pytest.raises(elements.IdentifierError, match="ID is invalid"):
        elements.find_element(target, basic_program)


def test_find_element_finds_element_in_function(basic_program: Program) -> None:
    expected_id = 1
    target = [2, 1]

    actual = elements.find_element(target, basic_program)

    assert isinstance(actual, elements.BinaryOperator)
    assert expected_id == actual.id
    assert elements.Operator.PLUS == actual.op

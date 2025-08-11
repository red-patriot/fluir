from editor.models import elements
from editor.utility.next_id import next_id


def test_empty_function() -> None:
    func = elements.Function(nodes=[], conduits=[])
    assert next_id(func) == 1


def test_function_with_nodes() -> None:
    nodes: elements.Nodes = [elements.Constant(id=1), elements.Constant(id=3)]
    func = elements.Function(nodes=nodes, conduits=[])
    assert next_id(func) == 4


def test_function_with_conduits() -> None:
    conduits = [elements.Conduit(id=2), elements.Conduit(id=5)]
    func = elements.Function(nodes=[], conduits=conduits)
    assert next_id(func) == 6


def test_function_with_nodes_and_conduits() -> None:
    nodes: elements.Nodes = [
        elements.BinaryOperator(id=1),
        elements.Constant(id=3),
    ]
    conduits = [elements.Conduit(id=2), elements.Conduit(id=5)]
    func = elements.Function(nodes=nodes, conduits=conduits)
    assert next_id(func) == 6

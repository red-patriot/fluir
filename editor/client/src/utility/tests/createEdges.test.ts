import { describe, it, expect } from 'vitest';
import { createEdges } from '../createNodes';
import FluirModule, {
  Conduit,
  FunctionDecl,
} from '../../models/fluir_module';

function makeFunc(
  overrides: Partial<FunctionDecl> & { id: number },
): FunctionDecl {
  return {
    discriminator: 'function',
    name: 'f',
    location: { x: 0, y: 0, z: 0, width: 100, height: 100 },
    nodes: [],
    conduits: [],
    ...overrides,
  };
}

function makeConduit(
  overrides: Partial<Conduit> & { id: number; input: number },
): Conduit {
  return {
    discriminator: 'conduit',
    children: [],
    ...overrides,
  };
}

function makeOutput(
  target: number,
  index: number,
): Conduit.Output {
  return { discriminator: 'conduit_output', target, index };
}

function makeSegment(
  x: number,
  y: number,
  children: (Conduit.Segment | Conduit.Output)[] = [],
): Conduit.Segment {
  return { discriminator: 'conduit_segment', x, y, children };
}

describe('createEdges', () => {
  describe('with empty module', () => {
    it('should return empty array for module with no declarations', () => {
      const module: FluirModule = { declarations: [] };

      const result = createEdges(module);

      expect(result).toEqual([]);
    });
  });

  describe('with function with no conduits', () => {
    it('should return empty array', () => {
      const module: FluirModule = {
        declarations: [makeFunc({ id: 1 })],
      };

      const result = createEdges(module);

      expect(result).toEqual([]);
    });
  });

  describe('with single conduit and single output', () => {
    const func = makeFunc({
      id: 1,
      conduits: [
        makeConduit({
          id: 10,
          input: 2,
          children: [makeOutput(3, 0)],
        }),
      ],
    });
    const module: FluirModule = { declarations: [func] };

    it('should create one edge', () => {
      const result = createEdges(module);

      expect(result).toHaveLength(1);
    });

    it('should set edge id to funcId:conduitId', () => {
      const result = createEdges(module);

      expect(result[0].id).toBe('1:10');
    });

    it('should set source to funcId:conduit.input', () => {
      const result = createEdges(module);

      expect(result[0].source).toBe('1:2');
    });

    it('should set target to funcId:child.target', () => {
      const result = createEdges(module);

      expect(result[0].target).toBe('1:3');
    });

    it('should set sourceHandle with input prefix and index 0', () => {
      const result = createEdges(module);

      expect(result[0].sourceHandle).toBe('input-1:2-0');
    });

    it('should set targetHandle with output prefix and child index', () => {
      const result = createEdges(module);

      expect(result[0].targetHandle).toBe('output-1:3-0');
    });

    it('should set type to conduit and animated to false', () => {
      const result = createEdges(module);

      expect(result[0].type).toBe('conduit');
      expect(result[0].animated).toBe(false);
    });

    it('should set strokeWidth to 2', () => {
      const result = createEdges(module);

      expect(result[0].style).toEqual({ strokeWidth: 2 });
    });

    it('should match full edge object', () => {
      const result = createEdges(module);

      expect(result[0]).toEqual({
        id: '1:10',
        source: '1:2',
        target: '1:3',
        sourceHandle: 'input-1:2-0',
        targetHandle: 'output-1:3-0',
        type: 'conduit',
        animated: false,
        style: { strokeWidth: 2 },
      });
    });
  });

  describe('with single conduit and multiple outputs', () => {
    it('should create one edge per conduit_output child', () => {
      const func = makeFunc({
        id: 1,
        conduits: [
          makeConduit({
            id: 10,
            input: 2,
            children: [makeOutput(3, 0), makeOutput(4, 1)],
          }),
        ],
      });
      const module: FluirModule = { declarations: [func] };

      const result = createEdges(module);

      expect(result).toHaveLength(2);
      expect(result[0].id).toBe('1:10');
      expect(result[0].target).toBe('1:3');
      expect(result[0].targetHandle).toBe('output-1:3-0');
      expect(result[1].id).toBe('1:10');
      expect(result[1].target).toBe('1:4');
      expect(result[1].targetHandle).toBe('output-1:4-1');
    });
  });

  describe('with conduit containing segments', () => {
    it('should ignore conduit_segment children', () => {
      const func = makeFunc({
        id: 1,
        conduits: [
          makeConduit({
            id: 10,
            input: 2,
            children: [
              makeSegment(5, 10),
              makeOutput(3, 0),
              makeSegment(15, 20, [makeOutput(99, 0)]),
            ],
          }),
        ],
      });
      const module: FluirModule = { declarations: [func] };

      const result = createEdges(module);

      expect(result).toHaveLength(1);
      expect(result[0].target).toBe('1:3');
    });
  });

  describe('with multiple conduits in one function', () => {
    it('should create edges for each conduit independently', () => {
      const func = makeFunc({
        id: 1,
        conduits: [
          makeConduit({
            id: 10,
            input: 2,
            children: [makeOutput(3, 0)],
          }),
          makeConduit({
            id: 11,
            input: 4,
            children: [makeOutput(5, 1)],
          }),
        ],
      });
      const module: FluirModule = { declarations: [func] };

      const result = createEdges(module);

      expect(result).toHaveLength(2);
      expect(result[0]).toEqual(
        expect.objectContaining({
          id: '1:10',
          source: '1:2',
          target: '1:3',
          sourceHandle: 'input-1:2-0',
          targetHandle: 'output-1:3-0',
        }),
      );
      expect(result[1]).toEqual(
        expect.objectContaining({
          id: '1:11',
          source: '1:4',
          target: '1:5',
          sourceHandle: 'input-1:4-0',
          targetHandle: 'output-1:5-1',
        }),
      );
    });
  });

  describe('with multiple function declarations', () => {
    it('should scope edge IDs to each function', () => {
      const func1 = makeFunc({
        id: 1,
        conduits: [
          makeConduit({
            id: 10,
            input: 2,
            children: [makeOutput(3, 0)],
          }),
        ],
      });
      const func2 = makeFunc({
        id: 5,
        conduits: [
          makeConduit({
            id: 20,
            input: 6,
            children: [makeOutput(7, 0)],
          }),
        ],
      });
      const module: FluirModule = {
        declarations: [func1, func2],
      };

      const result = createEdges(module);

      expect(result).toHaveLength(2);
      expect(result[0].id).toBe('1:10');
      expect(result[0].source).toBe('1:2');
      expect(result[0].target).toBe('1:3');
      expect(result[1].id).toBe('5:20');
      expect(result[1].source).toBe('5:6');
      expect(result[1].target).toBe('5:7');
    });
  });

  describe('with varied index values', () => {
    it('should correctly use child.index in targetHandle', () => {
      const func = makeFunc({
        id: 1,
        conduits: [
          makeConduit({
            id: 10,
            input: 2,
            children: [makeOutput(3, 5)],
          }),
        ],
      });
      const module: FluirModule = { declarations: [func] };

      const result = createEdges(module);

      expect(result[0].targetHandle).toBe('output-1:3-5');
    });

    it('should always use 0 for sourceHandle index', () => {
      const func = makeFunc({
        id: 1,
        conduits: [
          makeConduit({
            id: 10,
            input: 2,
            children: [makeOutput(3, 7)],
          }),
        ],
      });
      const module: FluirModule = { declarations: [func] };

      const result = createEdges(module);

      expect(result[0].sourceHandle).toBe('input-1:2-0');
    });
  });

  describe('with conduit with no output children', () => {
    it('should return empty array when conduit has only segments', () => {
      const func = makeFunc({
        id: 1,
        conduits: [
          makeConduit({
            id: 10,
            input: 2,
            children: [makeSegment(5, 10), makeSegment(15, 20)],
          }),
        ],
      });
      const module: FluirModule = { declarations: [func] };

      const result = createEdges(module);

      expect(result).toEqual([]);
    });
  });
});

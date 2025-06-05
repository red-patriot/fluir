import { describe, it, expect } from 'vitest';
import createNodes from '../createNodes';
import FluirModule, {
  BinaryOp,
  UnaryOp,
  FunctionDecl,
  Constant,
  Operator,
} from '../../models/fluir_module';
import { ZOOM_SCALAR } from '../../hooks/useSizeStyle';

describe('createNodes', () => {
  describe('with empty module', () => {
    it('should return empty array for module with no declarations', () => {
      const module: FluirModule = {
        declarations: [],
      };

      const result = createNodes(module);

      expect(result).toEqual([]);
    });
  });

  describe('with constant nodes', () => {
    it('should create constant node with correct properties', () => {
      const constant: Constant = {
        discriminator: 'constant',
        id: 1,
        location: { x: 100, y: 200, z: 0, width: 80, height: 40 },
        flType: 'FLOATING_POINT',
        value: '42.5',
      };

      const module: FluirModule = {
        declarations: [],
      };

      // Since constants are not declarations, we need to test them through function nodes
      const func: FunctionDecl = {
        discriminator: 'function',
        name: 'testFunc',
        id: 1,
        location: { x: 0, y: 0, z: 0, width: 200, height: 200 },
        nodes: [constant],
        conduits: [],
      };

      module.declarations = [func];

      const result = createNodes(module);

      expect(result).toHaveLength(3); // function + constant

      // Check the constant node (second in array)
      expect(result[2]).toEqual({
        type: 'constant',
        id: '1:1',
        parentId: '1',
        extent: 'parent',
        position: {
          x: 100 * ZOOM_SCALAR,
          y: 200 * ZOOM_SCALAR,
        },
        data: {
          constant: constant,
          fullID: '1:1',
        },
        dragHandle: '.dragHandle__custom',
      });
    });

    it('should handle constant with optional properties', () => {
      const constant: Constant = {
        discriminator: 'constant',
        id: 2,
        location: { x: 50, y: 100, z: 0, width: 60, height: 30 },
        // flType and value are optional
      };

      const func: FunctionDecl = {
        discriminator: 'function',
        name: 'testFunc',
        id: 1,
        location: { x: 0, y: 0, z: 0, width: 200, height: 200 },
        nodes: [constant],
        conduits: [],
      };

      const module: FluirModule = {
        declarations: [func],
      };

      const result = createNodes(module);

      expect(result).toHaveLength(3);
      expect(result[2].data.constant).toEqual(constant);
    });
  });

  describe('with binary operation nodes', () => {
    it('should create binary node with correct properties', () => {
      const binary: BinaryOp = {
        discriminator: 'binary',
        id: 3,
        location: { x: 75, y: 125, z: 0, width: 100, height: 50 },
        op: '+',
      };

      const func: FunctionDecl = {
        discriminator: 'function',
        name: 'testFunc',
        id: 1,
        location: { x: 0, y: 0, z: 0, width: 300, height: 300 },
        nodes: [binary],
        conduits: [],
      };

      const module: FluirModule = {
        declarations: [func],
      };

      const result = createNodes(module);

      expect(result).toHaveLength(3);
      expect(result[2]).toEqual({
        type: 'binary',
        id: '1:3',
        parentId: '1',
        extent: 'parent',
        position: {
          x: 75 * ZOOM_SCALAR,
          y: 125 * ZOOM_SCALAR,
        },
        data: {
          binary: binary,
          fullID: '1:3',
        },
        dragHandle: '.dragHandle__custom',
      });
    });

    it('should handle different operators', () => {
      const operators: Operator[] = ['+', '-', '*', '/'];

      const func: FunctionDecl = {
        discriminator: 'function',
        name: 'testFunc',
        id: 1,
        location: { x: 0, y: 0, z: 0, width: 300, height: 300 },
        nodes: operators.map((op, index) => ({
          discriminator: 'binary' as const,
          id: index + 1,
          location: { x: index * 50, y: 0, z: 0, width: 80, height: 40 },
          op: op,
        })),
        conduits: [],
      };

      const module: FluirModule = {
        declarations: [func],
      };

      const result = createNodes(module);

      expect(result).toHaveLength(6); // function + 5 binary nodes
      operators.forEach((op, index) => {
        expect(result[index + 2].data.binary.op).toBe(op);
      });
    });
  });

  describe('with unary operation nodes', () => {
    it('should create unary node with correct properties', () => {
      const unary: UnaryOp = {
        discriminator: 'unary',
        id: 4,
        location: { x: 300, y: 400, z: 0, width: 60, height: 40 },
        op: '-',
      };

      const func: FunctionDecl = {
        discriminator: 'function',
        name: 'testFunc',
        id: 1,
        location: { x: 0, y: 0, z: 0, width: 500, height: 500 },
        nodes: [unary],
        conduits: [],
      };

      const module: FluirModule = {
        declarations: [func],
      };

      const result = createNodes(module);

      expect(result).toHaveLength(3);
      expect(result[2]).toEqual({
        type: 'unary',
        id: '1:4',
        parentId: '1',
        extent: 'parent',
        position: {
          x: 300 * ZOOM_SCALAR,
          y: 400 * ZOOM_SCALAR,
        },
        data: {
          unary: unary,
          fullID: '1:4',
        },
        dragHandle: '.dragHandle__custom',
      });
    });
  });

  describe('with function nodes', () => {
    it('should create function node with no nested nodes', () => {
      const func: FunctionDecl = {
        discriminator: 'function',
        id: 5,
        location: { x: 50, y: 75, z: 0, width: 200, height: 150 },
        name: 'testFunction',
        nodes: [],
        conduits: [],
      };

      const module: FluirModule = {
        declarations: [func],
      };

      const result = createNodes(module);

      expect(result).toHaveLength(2);
      expect(result[1]).toEqual({
        type: 'function',
        id: '5',
        parentId: '5__header',
        extent: 'parent',
        position: {
          x: 0 * ZOOM_SCALAR,
          y: 4 * ZOOM_SCALAR,
        },
        data: {
          decl: func,
          fullID: '5',
        },
        dragHandle: '.dragHandle__custom',
      });
    });

    it('should create function node with nested nodes', () => {
      const nestedConstant: Constant = {
        discriminator: 'constant',
        id: 6,
        location: { x: 10, y: 20, z: 0, width: 80, height: 40 },
        flType: 'FLOATING_POINT',
        value: '100.0',
      };

      const nestedBinary: BinaryOp = {
        discriminator: 'binary',
        id: 7,
        location: { x: 30, y: 40, z: 0, width: 100, height: 50 },
        op: '*',
      };

      const func: FunctionDecl = {
        discriminator: 'function',
        id: 5,
        location: { x: 50, y: 75, z: 0, width: 300, height: 200 },
        name: 'complexFunction',
        nodes: [nestedConstant, nestedBinary],
        conduits: [],
      };

      const module: FluirModule = {
        declarations: [func],
      };

      const result = createNodes(module);

      expect(result).toHaveLength(4); // function + 2 nested nodes

      // Check function node
      expect(result[1]).toEqual({
        type: 'function',
        id: '5',
        parentId: '5__header',
        extent: 'parent',
        position: {
          x: 0 * ZOOM_SCALAR,
          y: 4 * ZOOM_SCALAR,
        },
        data: {
          decl: func,
          fullID: '5',
        },
        dragHandle: '.dragHandle__custom',
      });

      // Check nested constant
      expect(result[2]).toEqual({
        type: 'constant',
        id: '5:6',
        parentId: '5',
        extent: 'parent',
        position: {
          x: 10 * ZOOM_SCALAR,
          y: 20 * ZOOM_SCALAR,
        },
        data: {
          constant: nestedConstant,
          fullID: '5:6',
        },
        dragHandle: '.dragHandle__custom',
      });

      // Check nested binary
      expect(result[3]).toEqual({
        type: 'binary',
        id: '5:7',
        parentId: '5',
        extent: 'parent',
        position: {
          x: 30 * ZOOM_SCALAR,
          y: 40 * ZOOM_SCALAR,
        },
        data: {
          binary: nestedBinary,
          fullID: '5:7',
        },
        dragHandle: '.dragHandle__custom',
      });
    });

    describe('with mixed node types', () => {
      it('should handle function with all node types', () => {
        const constant: Constant = {
          discriminator: 'constant',
          id: 1,
          location: { x: 0, y: 0, z: 0, width: 80, height: 40 },
          flType: 'FLOATING_POINT',
          value: '42.0',
        };

        const binary: BinaryOp = {
          discriminator: 'binary',
          id: 2,
          location: { x: 50, y: 50, z: 0, width: 100, height: 50 },
          op: '+',
        };

        const unary: UnaryOp = {
          discriminator: 'unary',
          id: 3,
          location: { x: 100, y: 100, z: 0, width: 60, height: 40 },
          op: '-',
        };

        const func: FunctionDecl = {
          discriminator: 'function',
          id: 4,
          location: { x: 150, y: 150, z: 0, width: 300, height: 250 },
          name: 'mixedFunction',
          nodes: [constant, binary, unary],
          conduits: [],
        };

        const module: FluirModule = {
          declarations: [func],
        };

        const result = createNodes(module);

        expect(result).toHaveLength(5); // function + 3 nodes
        expect(result[0].type).toBe('function__header');
        expect(result[1].type).toBe('function');
        expect(result[2].type).toBe('constant');
        expect(result[3].type).toBe('binary');
        expect(result[4].type).toBe('unary');
        expect(result.map((n) => n.id)).toEqual([
          '4__header',
          '4',
          '4:1',
          '4:2',
          '4:3',
        ]);
      });

      it('should handle multiple functions', () => {
        const func1: FunctionDecl = {
          discriminator: 'function',
          id: 1,
          location: { x: 0, y: 0, z: 0, width: 200, height: 150 },
          name: 'function1',
          nodes: [],
          conduits: [],
        };

        const func2: FunctionDecl = {
          discriminator: 'function',
          id: 2,
          location: { x: 250, y: 0, z: 0, width: 200, height: 150 },
          name: 'function2',
          nodes: [],
          conduits: [],
        };

        const module: FluirModule = {
          declarations: [func1, func2],
        };

        const result = createNodes(module);

        expect(result).toHaveLength(4);
        expect(result[0].id).toBe('1__header');
        expect(result[1].id).toBe('1');
        expect(result[2].id).toBe('2__header');
        expect(result[3].id).toBe('2');
        expect(result[1].data.decl.name).toBe('function1');
        expect(result[2].data.decl.name).toBe('function2');
      });
    });

    describe('position scaling', () => {
      it('should correctly apply ZOOM_SCALAR to positions', () => {
        const constant: Constant = {
          discriminator: 'constant',
          id: 1,
          location: { x: 10, y: 20, z: 0, width: 80, height: 40 },
          value: '1',
        };

        const module: FluirModule = {
          declarations: [
            {
              discriminator: 'function',
              id: 2,
              location: { x: 0, y: 0, z: 0, width: 200, height: 200 },
              name: 'testFunction',
              nodes: [constant],
              conduits: [],
            },
          ],
        };

        const result = createNodes(module);

        expect(result[1].position).toEqual({
          x: 0 * ZOOM_SCALAR,
          y: 4 * ZOOM_SCALAR,
        });
      });

      it('should handle zero coordinates', () => {
        const constant: Constant = {
          discriminator: 'constant',
          id: 1,
          location: { x: 0, y: 0, z: 0, width: 0, height: 0 },
          value: '1',
        };

        const module: FluirModule = {
          declarations: [
            {
              discriminator: 'function',
              id: 2,
              location: { x: 0, y: 0, z: 0, width: 200, height: 200 },
              name: 'testFunction',
              nodes: [constant],
              conduits: [],
            },
          ],
        };

        const result = createNodes(module);

        expect(result[0].position).toEqual({
          x: 0,
          y: 0,
        });
      });

      it('should handle negative coordinates', () => {
        const constant: Constant = {
          discriminator: 'constant',
          id: 1,
          location: { x: -50, y: -100, z: 0, width: 80, height: 40 },
          value: '2.3',
        };

        const module: FluirModule = {
          declarations: [
            {
              discriminator: 'function',
              id: 2,
              location: { x: 0, y: 0, z: 0, width: 200, height: 200 },
              name: 'testFunction',
              nodes: [constant],
              conduits: [],
            },
          ],
        };

        const result = createNodes(module);

        expect(result[1].position).toEqual({
          x: 0 * ZOOM_SCALAR,
          y: 4 * ZOOM_SCALAR,
        });
      });
    });

    describe('id generation', () => {
      it('should generate correct ids for top-level nodes', () => {
        const module: FluirModule = {
          declarations: [
            {
              discriminator: 'function',
              id: 2,
              location: { x: 0, y: 0, z: 0, width: 200, height: 200 },
              name: 'testFunction',
              nodes: [],
              conduits: [],
            },
          ],
        };

        const result = createNodes(module);

        expect(result[1].id).toBe('2');
        expect(result[1].data.fullID).toBe('2');
      });

      it('should generate correct ids for nested nodes', () => {
        const nestedConstant: Constant = {
          discriminator: 'constant',
          id: 456,
          location: { x: 0, y: 0, z: 0, width: 80, height: 40 },
          value: '1',
        };

        const func: FunctionDecl = {
          discriminator: 'function',
          id: 123,
          location: { x: 0, y: 0, z: 0, width: 200, height: 200 },
          name: 'test',
          nodes: [nestedConstant],
          conduits: [],
        };

        const module: FluirModule = {
          declarations: [func],
        };

        const result = createNodes(module);

        expect(result[1].id).toBe('123'); // function
        expect(result[2].id).toBe('123:456'); // nested constant
        expect(result[2].parentId).toBe('123');
        expect(result[2].data.fullID).toBe('123:456');
      });
    });
  });
});

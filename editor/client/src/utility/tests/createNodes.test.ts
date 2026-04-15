import { describe, it, expect } from 'vitest';
import createNodes from '../createNodes';
import FluirModule, {
  BinaryOp,
  UnaryOp,
  FunctionDecl,
  FunctionParameter,
  FunctionReturn,
  Constant,
  Operator,
  Call,
} from '../../models/fluir_module';
import { ZOOM_SCALAR } from '../../hooks/useSizeStyle';
import { FUNC_HEADER_HEIGHT } from '@/components/flow_diagram/elements/FunctionDeclNode';

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
    it('should create f64 node with correct properties', () => {
      const constant: Constant = {
        discriminator: 'constant',
        id: 1,
        location: { x: 100, y: 200, z: 0, width: 80, height: 40 },
        flType: 'F64',
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
        inputs: [],
        outputs: [],
      };

      module.declarations = [func];

      const result = createNodes(module);

      expect(result).toHaveLength(2); // function + constant

      // Check the constant node (second in array)
      expect(result[1]).toEqual({
        type: 'constant',
        id: '1:1',
        parentId: '1',
        extent: [
          [0, FUNC_HEADER_HEIGHT * ZOOM_SCALAR],
          [200 * ZOOM_SCALAR, 200 * ZOOM_SCALAR],
        ],
        position: {
          x: 100 * ZOOM_SCALAR,
          y: 200 * ZOOM_SCALAR,
        },
        width: 80 * ZOOM_SCALAR,
        height: 40 * ZOOM_SCALAR,
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
        inputs: [],
        outputs: [],
      };

      const module: FluirModule = {
        declarations: [func],
      };

      const result = createNodes(module);

      expect(result).toHaveLength(2);
      expect(result[1].data.constant).toEqual(constant);
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
        inputs: [],
        outputs: [],
      };

      const module: FluirModule = {
        declarations: [func],
      };

      const result = createNodes(module);

      expect(result).toHaveLength(2);
      expect(result[1]).toEqual({
        type: 'binary',
        id: '1:3',
        parentId: '1',
        extent: [
          [0, FUNC_HEADER_HEIGHT * ZOOM_SCALAR],
          [300 * ZOOM_SCALAR, 300 * ZOOM_SCALAR],
        ],
        position: {
          x: 75 * ZOOM_SCALAR,
          y: 125 * ZOOM_SCALAR,
        },
        width: 100 * ZOOM_SCALAR,
        height: 50 * ZOOM_SCALAR,
        data: {
          operator: binary,
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
        inputs: [],
        outputs: [],
      };

      const module: FluirModule = {
        declarations: [func],
      };

      const result = createNodes(module);

      expect(result).toHaveLength(5); // function + 5 binary nodes
      operators.forEach((op, index) => {
        expect(result[index + 1].data.operator.op).toBe(op);
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
        inputs: [],
        outputs: [],
      };

      const module: FluirModule = {
        declarations: [func],
      };

      const result = createNodes(module);

      expect(result).toHaveLength(2);
      expect(result[1]).toEqual({
        type: 'unary',
        id: '1:4',
        parentId: '1',
        extent: [
          [0, FUNC_HEADER_HEIGHT * ZOOM_SCALAR],
          [500 * ZOOM_SCALAR, 500 * ZOOM_SCALAR],
        ],
        position: {
          x: 300 * ZOOM_SCALAR,
          y: 400 * ZOOM_SCALAR,
        },
        width: 60 * ZOOM_SCALAR,
        height: 40 * ZOOM_SCALAR,
        data: {
          operator: unary,
          fullID: '1:4',
        },
        dragHandle: '.dragHandle__custom',
      });
    });
  });

  describe('with call nodes', () => {
    it('should create call node with correct properties', () => {
      const call: Call = {
        discriminator: 'call',
        id: 8,
        target: 'otherFunc',
        location: { x: 120, y: 160, z: 0, width: 90, height: 60 },
        arguments: ['a', 'b'],
        returns: true,
      };

      const func: FunctionDecl = {
        discriminator: 'function',
        name: 'testFunc',
        id: 1,
        location: { x: 0, y: 0, z: 0, width: 400, height: 400 },
        nodes: [call],
        conduits: [],
        inputs: [],
        outputs: [],
      };

      const result = createNodes({ declarations: [func] });

      expect(result).toHaveLength(2);
      expect(result[1]).toEqual({
        type: 'call',
        id: '1:8',
        parentId: '1',
        extent: [
          [0, FUNC_HEADER_HEIGHT * ZOOM_SCALAR],
          [400 * ZOOM_SCALAR, 400 * ZOOM_SCALAR],
        ],
        position: {
          x: 120 * ZOOM_SCALAR,
          y: 160 * ZOOM_SCALAR,
        },
        width: 90 * ZOOM_SCALAR,
        height: 60 * ZOOM_SCALAR,
        data: {
          call: call,
          fullID: '1:8',
        },
        dragHandle: '.dragHandle__custom',
      });
    });

    it('should pass the Call object by reference on data.call', () => {
      const call: Call = {
        discriminator: 'call',
        id: 9,
        target: 'doThing',
        location: { x: 0, y: 0, z: 0, width: 50, height: 50 },
        arguments: ['x'],
        returns: false,
      };

      const func: FunctionDecl = {
        discriminator: 'function',
        name: 'testFunc',
        id: 1,
        location: { x: 0, y: 0, z: 0, width: 200, height: 200 },
        nodes: [call],
        conduits: [],
        inputs: [],
        outputs: [],
      };

      const result = createNodes({ declarations: [func] });

      expect(result[1].data.call).toBe(call);
      expect(result[1].data.call.target).toBe('doThing');
      expect(result[1].data.call.arguments).toEqual(['x']);
      expect(result[1].data.call.returns).toBe(false);
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
        inputs: [],
        outputs: [],
      };

      const module: FluirModule = {
        declarations: [func],
      };

      const result = createNodes(module);

      expect(result).toHaveLength(1);
      expect(result[0]).toEqual({
        type: 'function',
        id: '5',
        height: 150 * ZOOM_SCALAR,
        width: 200 * ZOOM_SCALAR,
        position: {
          x: 50 * ZOOM_SCALAR,
          y: 75 * ZOOM_SCALAR,
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
        flType: 'F64',
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
        inputs: [],
        outputs: [],
      };

      const module: FluirModule = {
        declarations: [func],
      };

      const result = createNodes(module);

      expect(result).toHaveLength(3); // function + 2 nested nodes

      // Check function node
      expect(result[0]).toEqual({
        type: 'function',
        id: '5',
        width: 300 * ZOOM_SCALAR,
        height: 200 * ZOOM_SCALAR,
        position: {
          x: 50 * ZOOM_SCALAR,
          y: 75 * ZOOM_SCALAR,
        },
        data: {
          decl: func,
          fullID: '5',
        },
        dragHandle: '.dragHandle__custom',
      });

      // Check nested constant
      expect(result[1]).toEqual({
        type: 'constant',
        id: '5:6',
        parentId: '5',
        extent: [
          [0, FUNC_HEADER_HEIGHT * ZOOM_SCALAR],
          [300 * ZOOM_SCALAR, 200 * ZOOM_SCALAR],
        ],
        position: {
          x: 10 * ZOOM_SCALAR,
          y: 20 * ZOOM_SCALAR,
        },
        width: 80 * ZOOM_SCALAR,
        height: 40 * ZOOM_SCALAR,
        data: {
          constant: nestedConstant,
          fullID: '5:6',
        },
        dragHandle: '.dragHandle__custom',
      });

      // Check nested binary
      expect(result[2]).toEqual({
        type: 'binary',
        id: '5:7',
        parentId: '5',
        extent: [
          [0, FUNC_HEADER_HEIGHT * ZOOM_SCALAR],
          [300 * ZOOM_SCALAR, 200 * ZOOM_SCALAR],
        ],
        position: {
          x: 30 * ZOOM_SCALAR,
          y: 40 * ZOOM_SCALAR,
        },
        width: 100 * ZOOM_SCALAR,
        height: 50 * ZOOM_SCALAR,
        data: {
          operator: nestedBinary,
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
          flType: 'F64',
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

        const call: Call = {
          discriminator: 'call',
          id: 5,
          target: 'helper',
          location: { x: 150, y: 150, z: 0, width: 80, height: 50 },
          arguments: [],
          returns: true,
        };

        const func: FunctionDecl = {
          discriminator: 'function',
          id: 4,
          location: { x: 150, y: 150, z: 0, width: 300, height: 250 },
          name: 'mixedFunction',
          nodes: [constant, binary, unary, call],
          conduits: [],
          inputs: [],
          outputs: [],
        };

        const module: FluirModule = {
          declarations: [func],
        };

        const result = createNodes(module);

        expect(result).toHaveLength(5); // function + 4 nodes
        expect(result[0].type).toBe('function');
        expect(result[1].type).toBe('constant');
        expect(result[2].type).toBe('binary');
        expect(result[3].type).toBe('unary');
        expect(result[4].type).toBe('call');
        expect(result.map((n) => n.id)).toEqual([
          '4',
          '4:1',
          '4:2',
          '4:3',
          '4:5',
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
          inputs: [],
          outputs: [],
        };

        const func2: FunctionDecl = {
          discriminator: 'function',
          id: 2,
          location: { x: 250, y: 0, z: 0, width: 200, height: 150 },
          name: 'function2',
          nodes: [],
          conduits: [],
          inputs: [],
          outputs: [],
        };

        const module: FluirModule = {
          declarations: [func1, func2],
        };

        const result = createNodes(module);

        expect(result).toHaveLength(2);
        expect(result[0].id).toBe('1');
        expect(result[1].id).toBe('2');
        expect(result[0].data.decl.name).toBe('function1');
        expect(result[1].data.decl.name).toBe('function2');
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
              inputs: [],
              outputs: [],
            },
          ],
        };

        const result = createNodes(module);

        expect(result[1].position).toEqual({
          x: 10 * ZOOM_SCALAR,
          y: 20 * ZOOM_SCALAR,
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
              inputs: [],
              outputs: [],
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
              inputs: [],
              outputs: [],
            },
          ],
        };

        const result = createNodes(module);

        expect(result[1].position).toEqual({
          x: -50 * ZOOM_SCALAR,
          y: -100 * ZOOM_SCALAR,
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
              inputs: [],
              outputs: [],
            },
          ],
        };

        const result = createNodes(module);

        expect(result[0].id).toBe('2');
        expect(result[0].data.fullID).toBe('2');
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
          inputs: [],
          outputs: [],
        };

        const module: FluirModule = {
          declarations: [func],
        };

        const result = createNodes(module);

        expect(result[0].id).toBe('123'); // function
        expect(result[1].id).toBe('123:456'); // nested constant
        expect(result[1].parentId).toBe('123');
        expect(result[1].data.fullID).toBe('123:456');
      });
    });
  });

  describe('with function parameters and returns', () => {
    it('should emit no parameter or return nodes when inputs and outputs are empty', () => {
      const func: FunctionDecl = {
        discriminator: 'function',
        name: 'testFunc',
        id: 1,
        location: { x: 0, y: 0, z: 0, width: 200, height: 200 },
        nodes: [],
        conduits: [],
        inputs: [],
        outputs: [],
      };

      const result = createNodes({ declarations: [func] });

      expect(result).toHaveLength(1);
      expect(result[0].type).toBe('function');
    });

    it('should create a parameter node with correct shape', () => {
      const parameter: FunctionParameter = {
        id: 10,
        name: 'a',
        flType: 'I32',
      };

      const func: FunctionDecl = {
        discriminator: 'function',
        name: 'testFunc',
        id: 1,
        location: { x: 0, y: 0, z: 0, width: 200, height: 200 },
        nodes: [],
        conduits: [],
        inputs: [parameter],
        outputs: [],
      };

      const result = createNodes({ declarations: [func] });

      expect(result).toHaveLength(2);
      expect(result[1]).toEqual({
        type: 'parameter',
        id: '1:10',
        parentId: '1',
        extent: undefined,
        position: {
          x: 0,
          y: 5 * ZOOM_SCALAR,
        },
        width: 12 * ZOOM_SCALAR,
        height: 5 * ZOOM_SCALAR,
        data: {
          funcID: '1',
          fullID: '1:10',
          parameter: parameter,
        },
        dragHandle: '.dragHandle__custom',
      });
    });

    it('should stack multiple parameters vertically by index', () => {
      const params: FunctionParameter[] = [
        { id: 11, name: 'a', flType: 'I32' },
        { id: 12, name: 'b', flType: 'F64' },
        { id: 13, name: 'c', flType: 'U8' },
      ];

      const func: FunctionDecl = {
        discriminator: 'function',
        name: 'testFunc',
        id: 1,
        location: { x: 0, y: 0, z: 0, width: 200, height: 200 },
        nodes: [],
        conduits: [],
        inputs: params,
        outputs: [],
      };

      const result = createNodes({ declarations: [func] });

      expect(result).toHaveLength(4);
      expect(result[1].id).toBe('1:11');
      expect(result[1].position.y).toBe(1 * 5 * ZOOM_SCALAR);
      expect(result[2].id).toBe('1:12');
      expect(result[2].position.y).toBe(2 * 5 * ZOOM_SCALAR);
      expect(result[3].id).toBe('1:13');
      expect(result[3].position.y).toBe(3 * 5 * ZOOM_SCALAR);
      // x should always be 0 for parameters
      expect(result[1].position.x).toBe(0);
      expect(result[2].position.x).toBe(0);
      expect(result[3].position.x).toBe(0);
    });

    it('should create a return node with correct shape', () => {
      const ret: FunctionReturn = {
        id: 20,
        flType: 'F64',
      };

      const func: FunctionDecl = {
        discriminator: 'function',
        name: 'testFunc',
        id: 1,
        location: { x: 0, y: 0, z: 0, width: 80, height: 100 },
        nodes: [],
        conduits: [],
        inputs: [],
        outputs: [ret],
      };

      const result = createNodes({ declarations: [func] });

      expect(result).toHaveLength(2);
      expect(result[1]).toEqual({
        type: 'return_',
        id: '1:20',
        parentId: '1',
        extent: undefined,
        position: {
          x: (80 - 5) * ZOOM_SCALAR,
          y: 5 * ZOOM_SCALAR,
        },
        width: 5 * ZOOM_SCALAR,
        height: 5 * ZOOM_SCALAR,
        data: {
          funcID: '1',
          fullID: '1:20',
          return_: ret,
        },
        dragHandle: '.dragHandle__custom',
      });
    });

    it('should stack multiple returns vertically by index at right edge', () => {
      const returns: FunctionReturn[] = [
        { id: 21, flType: 'I32' },
        { id: 22, flType: 'F64' },
      ];

      const func: FunctionDecl = {
        discriminator: 'function',
        name: 'testFunc',
        id: 1,
        location: { x: 0, y: 0, z: 0, width: 80, height: 100 },
        nodes: [],
        conduits: [],
        inputs: [],
        outputs: returns,
      };

      const result = createNodes({ declarations: [func] });

      expect(result).toHaveLength(3);
      expect(result[1].id).toBe('1:21');
      expect(result[1].position.y).toBe(1 * 5 * ZOOM_SCALAR);
      expect(result[1].position.x).toBe((80 - 5) * ZOOM_SCALAR);
      expect(result[2].id).toBe('1:22');
      expect(result[2].position.y).toBe(2 * 5 * ZOOM_SCALAR);
      expect(result[2].position.x).toBe((80 - 5) * ZOOM_SCALAR);
    });

    it('should emit parameters before returns before nested nodes', () => {
      const parameter: FunctionParameter = {
        id: 30,
        name: 'a',
        flType: 'I32',
      };
      const ret: FunctionReturn = {
        id: 40,
        flType: 'I32',
      };
      const nestedConstant: Constant = {
        discriminator: 'constant',
        id: 50,
        location: { x: 20, y: 30, z: 0, width: 80, height: 40 },
        flType: 'I32',
        value: '7',
      };

      const func: FunctionDecl = {
        discriminator: 'function',
        name: 'testFunc',
        id: 1,
        location: { x: 0, y: 0, z: 0, width: 200, height: 200 },
        nodes: [nestedConstant],
        conduits: [],
        inputs: [parameter],
        outputs: [ret],
      };

      const result = createNodes({ declarations: [func] });

      expect(result).toHaveLength(4);
      expect(result[0].type).toBe('function');
      expect(result[1].type).toBe('parameter');
      expect(result[1].id).toBe('1:30');
      expect(result[2].type).toBe('return_');
      expect(result[2].id).toBe('1:40');
      expect(result[3].type).toBe('constant');
      expect(result[3].id).toBe('1:50');
    });

    it('should use parameter and return ids (not array index) for qualified IDs', () => {
      const params: FunctionParameter[] = [
        { id: 7, name: 'a', flType: 'I32' },
        { id: 3, name: 'b', flType: 'I32' },
      ];
      const returns: FunctionReturn[] = [
        { id: 9, flType: 'I32' },
        { id: 4, flType: 'I32' },
      ];

      const func: FunctionDecl = {
        discriminator: 'function',
        name: 'testFunc',
        id: 1,
        location: { x: 0, y: 0, z: 0, width: 200, height: 200 },
        nodes: [],
        conduits: [],
        inputs: params,
        outputs: returns,
      };

      const result = createNodes({ declarations: [func] });

      expect(result[1].id).toBe('1:7');
      expect(result[2].id).toBe('1:3');
      expect(result[3].id).toBe('1:9');
      expect(result[4].id).toBe('1:4');
    });
  });
});

import FunctionDeclNode, {
  FUNC_HEADER_HEIGHT,
} from '@/components/flow_diagram/elements/FunctionDeclNode';
import ConstantNode from '@/components/flow_diagram/elements/ConstantNode';
import {
  BinaryOperatorNode,
  UnaryOperatorNode,
} from '@/components/flow_diagram/elements/OperatorNode';
import FunctionParameterNode from '@/components/flow_diagram/elements/FunctionParameterNode.tsx';
import FunctionReturnNode from '@/components/flow_diagram/elements/FunctionReturnNode.tsx';
import FluirModule, {
  BinaryOp,
  Constant,
  Declaration,
  FunctionDecl, FunctionParameter, FunctionReturn,
  Node,
  UnaryOp,
} from '@/models/fluir_module';
import { ZOOM_SCALAR } from '../hooks/useSizeStyle';
import { Edge, Node as FlowNode, CoordinateExtent } from '@xyflow/react';

function fullId(parentId: string | undefined, id: number): string {
  return parentId ? `${parentId}:${id}` : `${id}`;
}

const PARAM_BLOCK_HEIGHT = 5;

function addNodes(
  nodes: FlowNode[],
  item: Declaration | Node,
  parentId?: string,
  extent?: 'parent' | CoordinateExtent,
) {
  switch (item.discriminator) {
    case 'function':
      const decl = item as FunctionDecl;
      const id = fullId(parentId, decl.id);
      const childrenExtent: CoordinateExtent = [
        [0, FUNC_HEADER_HEIGHT * ZOOM_SCALAR],
        [decl.location.width * ZOOM_SCALAR, decl.location.height * ZOOM_SCALAR],
      ];
      nodes.push({
        type: 'function',
        id: id,
        parentId: parentId,
        extent: extent,
        position: {
          x: decl.location.x * ZOOM_SCALAR,
          y: decl.location.y * ZOOM_SCALAR,
        },
        width: decl.location.width * ZOOM_SCALAR,
        height: decl.location.height * ZOOM_SCALAR,
        data: {
          decl: decl,
          fullID: id,
        },
        dragHandle: '.dragHandle__custom',
      });
      decl.inputs.forEach(
        addFunctionParameterNode(nodes, id, extent),
      );
      decl.outputs.forEach(
        addFunctionReturnNode(nodes, id, decl, extent),
      );
      decl.nodes.forEach((node) => {
        addNodes(nodes, node, id, childrenExtent);
      });
      break;
    case 'constant':
      nodes.push({
        type: 'constant',
        id: fullId(parentId, item.id),
        parentId: parentId,
        extent: extent,
        position: {
          x: item.location.x * ZOOM_SCALAR,
          y: item.location.y * ZOOM_SCALAR,
        },
        width: item.location.width * ZOOM_SCALAR,
        height: item.location.height * ZOOM_SCALAR,
        data: {
          constant: item as Constant,
          fullID: fullId(parentId, item.id),
        },
        dragHandle: '.dragHandle__custom',
      });
      break;
    case 'binary':
      nodes.push({
        type: 'binary',
        id: fullId(parentId, item.id),
        parentId: parentId,
        extent: extent,
        position: {
          x: item.location.x * ZOOM_SCALAR,
          y: item.location.y * ZOOM_SCALAR,
        },
        width: item.location.width * ZOOM_SCALAR,
        height: item.location.height * ZOOM_SCALAR,
        data: {
          operator: item as BinaryOp,
          fullID: fullId(parentId, item.id),
        },
        dragHandle: '.dragHandle__custom',
      });
      break;
    case 'unary':
      nodes.push({
        type: 'unary',
        id: fullId(parentId, item.id),
        parentId: parentId,
        extent: extent,
        position: {
          x: item.location.x * ZOOM_SCALAR,
          y: item.location.y * ZOOM_SCALAR,
        },
        width: item.location.width * ZOOM_SCALAR,
        height: item.location.height * ZOOM_SCALAR,
        data: {
          operator: item as UnaryOp,
          fullID: fullId(parentId, item.id),
        },
        dragHandle: '.dragHandle__custom',
      });
      break;
  }
}

function addFunctionParameterNode(nodes: FlowNode[], funcID: string, extent?: 'parent' | CoordinateExtent) {
  return (param: FunctionParameter, index: number) => {
    const paramID = fullId(funcID, param.id);
    nodes.push({
      type: 'parameter',
      id: fullId(funcID, param.id),
      parentId: funcID,
      extent: extent,
      position: {
        x: 0 * ZOOM_SCALAR,
        y: (index + 1) * PARAM_BLOCK_HEIGHT * ZOOM_SCALAR,
      },
      width: 12 * ZOOM_SCALAR,
      height: PARAM_BLOCK_HEIGHT * ZOOM_SCALAR,
      data: {
        funcID: funcID,
        fullID: paramID,
        parameter: param,
      },
      dragHandle: '.dragHandle__custom',
    });
  };
}

function addFunctionReturnNode(nodes: FlowNode[], funcID: string, decl: FunctionDecl, extent?: 'parent' | CoordinateExtent) {
  return (ret: FunctionReturn, index: number) => {
    const retID = fullId(funcID, ret.id);
    nodes.push({
      type: 'return_',
      id: retID,
      parentId: funcID,
      extent: extent,
      position: {
        x: (decl.location.width - 5) * ZOOM_SCALAR,
        y: (index + 1) * PARAM_BLOCK_HEIGHT * ZOOM_SCALAR,
      },
      width: 5 * ZOOM_SCALAR,
      height: PARAM_BLOCK_HEIGHT * ZOOM_SCALAR,
      data: {
        funcID: funcID,
        fullID: retID,
        return_: ret,
      },
      dragHandle: '.dragHandle__custom',
    });
  };
}

export default function createNodes(module: FluirModule) {
  let nodes: any[] = [];
  module.declarations.forEach((decl) => {
    addNodes(nodes, decl);
  });
  return nodes;
}

export function createEdges(module: FluirModule) {
  let edges: Edge[] = [];
  module.declarations.forEach((decl) => {
    if (decl.discriminator === 'function') {
      const functionDecl = decl as FunctionDecl;
      functionDecl.conduits.forEach((conduit) => {
        conduit.children.forEach((child) => {
          if (child.discriminator === 'conduit_output') {
            edges.push({
              id: fullId(`${functionDecl.id}`, conduit.id),
              source: `${fullId(`${functionDecl.id}`, conduit.input)}`,
              target: `${fullId(`${functionDecl.id}`, child.target)}`,
              sourceHandle: `input-${fullId(
                `${functionDecl.id}`,
                conduit.input,
              )}-0`,
              targetHandle: `output-${fullId(
                `${functionDecl.id}`,
                child.target,
              )}-${child.index}`,
              type: 'straight',
              animated: false,
              style: { strokeWidth: 2 },
            });
          }
        });
      });
    }
  });
  return edges;
}

export const nodeTypes = {
  function: FunctionDeclNode,
  parameter: FunctionParameterNode,
  return_: FunctionReturnNode,
  constant: ConstantNode,
  binary: BinaryOperatorNode,
  unary: UnaryOperatorNode,
};

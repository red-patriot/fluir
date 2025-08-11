import BinaryNode from '../components/custom/BinaryNode';
import ConstantNode from '../components/custom/ConstantNode';
import FunctionDeclNode, {
  FUNC_HEADER_HEIGHT,
} from '../components/custom/FunctionDeclNode';
import UnaryNode from '../components/custom/UnaryNode';
import FluirModule, {
  BinaryOp,
  Constant,
  Declaration,
  FunctionDecl,
  Node,
  UnaryOp,
} from '../models/fluir_module';
import { ZOOM_SCALAR } from '../hooks/useSizeStyle';
import { Edge, Node as FlowNode, CoordinateExtent } from '@xyflow/react';

function fullId(parentId: string | undefined, id: number): string {
  return parentId ? `${parentId}:${id}` : `${id}`;
}

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
        measured: {
          width: decl.location.width * ZOOM_SCALAR,
          height: decl.location.height * ZOOM_SCALAR,
        },
        data: {
          decl: decl,
          fullID: id,
        },
        dragHandle: '.dragHandle__custom',
      });
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
        data: {
          binary: item as BinaryOp,
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
        data: {
          unary: item as UnaryOp,
          fullID: fullId(parentId, item.id),
        },
        dragHandle: '.dragHandle__custom',
      });
      break;
  }
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
  constant: ConstantNode,
  binary: BinaryNode,
  unary: UnaryNode,
};

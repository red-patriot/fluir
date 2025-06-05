import BinaryNode from '../components/custom/BinaryNode';
import ConstantNode from '../components/custom/ConstantNode';
import FunctionDeclNode, {
  FunctionDeclHeaderNode,
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
import { ZOOM_SCALAR, HEADER_HEIGHT } from '../hooks/useSizeStyle';
import { Edge } from '@xyflow/react';

function fullId(parentId: string | undefined, id: number): string {
  return parentId ? `${parentId}:${id}` : `${id}`;
}

function headerID(parentId: string | undefined, id: number): string {
  return `${fullId(parentId, id)}__header`;
}

function addNodes(nodes: any[], item: Declaration | Node, parentId?: string) {
  switch (item.discriminator) {
    case 'function':
      const decl = item as FunctionDecl;
      nodes.push({
        type: 'function__header',
        id: headerID(parentId, decl.id),
        parentId: parentId,
        extent: 'parent',
        position: {
          x: decl.location.x * ZOOM_SCALAR,
          y: decl.location.y * ZOOM_SCALAR,
        },
        data: {
          decl: decl,
          fullID: fullId(parentId, decl.id),
        },
        dragHandle: '.dragHandle__custom',
      });
      nodes.push({
        type: 'function',
        id: fullId(parentId, decl.id),
        parentId: headerID(parentId, decl.id),
        extent: 'parent',
        position: {
          x: 0,
          y: HEADER_HEIGHT * ZOOM_SCALAR,
        },
        data: {
          decl: decl,
          fullID: fullId(parentId, decl.id),
        },
        dragHandle: '.dragHandle__custom',
      });
      decl.nodes.forEach((node) => {
        addNodes(nodes, node, `${decl.id}`);
      });
      break;
    case 'constant':
      nodes.push({
        type: 'constant',
        id: fullId(parentId, item.id),
        parentId: parentId,
        extent: 'parent',
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
        extent: 'parent',
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
        extent: 'parent',
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
  function__header: FunctionDeclHeaderNode,
  constant: ConstantNode,
  binary: BinaryNode,
  unary: UnaryNode,
};

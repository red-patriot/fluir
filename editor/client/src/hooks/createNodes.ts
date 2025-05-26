import BinaryNode from '../components/custom/BinaryNode';
import ConstantNode from '../components/custom/ConstantNode';
import FunctionDeclNode from '../components/custom/FunctionDeclNode';
import UnaryNode from '../components/custom/UnaryNode';
import FluirModule, {
  BinaryOp,
  Constant,
  Declaration,
  FunctionDecl,
  Node,
  UnaryOp,
} from '../models/fluir_module';

function fullId(parentId: string | undefined, id: number): string {
  return parentId ? `${parentId}:${id}` : `${id}`;
}

function addNodes(nodes: any[], item: Declaration | Node, parentId?: string) {
  switch (item.discriminator) {
    case 'function':
      const decl = item as FunctionDecl;
      nodes.push({
        type: 'function',
        id: fullId(parentId, decl.id),
        parentId: parentId,
        extent: 'parent',
        position: { x: decl.location.x, y: decl.location.y },
        data: {
          decl: decl,
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
        position: { x: item.location.x, y: item.location.y },
        data: {
          constant: item as Constant,
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
        position: { x: item.location.x, y: item.location.y },
        data: {
          binary: item as BinaryOp,
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
        position: { x: item.location.x, y: item.location.y },
        data: {
          unary: item as UnaryOp,
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

export const nodeTypes = {
  function: FunctionDeclNode,
  constant: ConstantNode,
  binary: BinaryNode,
  unary: UnaryNode,
};

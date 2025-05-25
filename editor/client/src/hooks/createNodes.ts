import ConstantNode from '../components/custom/ConstantNode';
import FunctionDeclNode from '../components/custom/FunctionDeclNode';
import FluirModule, {
  Constant,
  Declaration,
  FunctionDecl,
  Node,
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
};

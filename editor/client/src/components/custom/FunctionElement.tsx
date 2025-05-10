import { FunctionDecl, Node } from '../../models/fluir_module';
import ConstantElement from './ConstantElement';
import OpBinaryElement from './OpBinaryElement';
import OpUnaryElement from './OpUnaryElement';

interface FunctionElementProps {
  decl: FunctionDecl;
}

export default function FunctionElement({ decl }: FunctionElementProps) {
  // TODO: Support other decl types
  const { x, y, z, width, height } = decl.location;

  const displayNodes = (node: Node) => {
    if (node._t === 'constant') {
      return (
        <ConstantElement
          key={node.id}
          constant={node}
        />
      );
    } else if (node._t === 'binary') {
      return (
        <OpBinaryElement
          key={node.id}
          binary={node}
        />
      );
    } else if (node._t === 'unary') {
      return (
        <OpUnaryElement
          key={node.id}
          unary={node}
        />
      );
    }
  };

  return (
    <div
      key={decl.id}
      className='absolute border-2 border-gray-200 '
      style={{
        left: `${x}px`,
        top: `${y}px`,
        width: `${width}px`,
        height: `${height}px`,
        zIndex: `${z}`,
      }}
    >
      <p className='w-full border border-gray-200'>{decl.name}</p>
      <div className='absolute'>{decl.body.map(displayNodes)}</div>
    </div>
  );
}

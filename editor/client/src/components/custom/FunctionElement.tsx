import { FunctionDecl, Node } from '../../models/fluir_module';
import ConstantElement from './ConstantElement';
import OpBinaryElement from './OpBinaryElement';
import OpUnaryElement from './OpUnaryElement';
import { useSizeStyle } from '../../hooks/useSizeStyle';

interface FunctionElementProps {
  decl: FunctionDecl;
}

export default function FunctionElement({ decl }: FunctionElementProps) {
  // Local functions
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

  const getSizeStyle = useSizeStyle(decl.location);

  return (
    <div
      aria-label={`func-${decl.name}-${decl.id}`}
      key={decl.id}
      className='absolute border-2 border-gray-200 '
      style={getSizeStyle()}
    >
      <p className='w-full border border-gray-200'>{decl.name}</p>
      <div className='absolute'>{decl.body.map(displayNodes)}</div>
    </div>
  );
}

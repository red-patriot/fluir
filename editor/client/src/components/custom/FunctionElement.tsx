import { FunctionDecl, Node } from '../../models/fluir_module';
import ConstantElement from './ConstantElement';
import OpBinaryElement from './OpBinaryElement';
import OpUnaryElement from './OpUnaryElement';
import { useSizeStyle } from '../../hooks/useSizeStyle';
import { useDraggable } from '@dnd-kit/core';

interface FunctionElementProps {
  decl: FunctionDecl;
}

export default function FunctionElement({ decl }: FunctionElementProps) {
  // Local state
  const { attributes, listeners, setNodeRef, transform } = useDraggable({
    id: `frunc-${decl.id}`,
  });

  const transformStyle = transform
    ? {
        transform: `translate3d(${transform.x}px, ${transform.y}px, 0)`,
      }
    : undefined;

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

  const { getSizeStyle, getFontSize } = useSizeStyle(decl.location);

  return (
    <div
      ref={setNodeRef}
      {...listeners}
      {...attributes}
      aria-label={`func-${decl.name}-${decl.id}`}
      key={decl.id}
      className='absolute border-1 rounded-sm border-gray-200
      bg-gray-950 font-code'
      style={{ ...getSizeStyle(), ...transformStyle }}
    >
      <p
        className='w-full border-b border-gray-200'
        style={{ ...getFontSize(24) }}
      >
        {decl.name}
      </p>
      <div className='absolute'>{decl.body.map(displayNodes)}</div>
    </div>
  );
}

import { FunctionDecl, Node } from '../../models/fluir_module';
import ConstantElement from './ConstantElement';
import OpBinaryElement from './OpBinaryElement';
import OpUnaryElement from './OpUnaryElement';
import { useSizeStyle, ZOOM_SCALAR } from '../../hooks/useSizeStyle';
import { useDraggable } from '@dnd-kit/core';
import { useAppSelector } from '../../store';

interface FunctionElementProps {
  decl: FunctionDecl;
}

export default function FunctionElement({ decl }: FunctionElementProps) {
  // GlobalState
  const zoom = useAppSelector((state) => state.program.zoom) * ZOOM_SCALAR;

  // Local state
  const { attributes, listeners, setNodeRef, transform } = useDraggable({
    id: `${decl.id}`,
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

  const getSizeStyle = () => {
    const { x, y, z, width, height } = decl.location;
    return {
      left: `${x * zoom}px`,
      top: `${y * zoom}px`,
      width: `${width * zoom}px`,
      height: `${height * zoom}px`,
      zIndex: `${z}`,
    };
  };

  const { getFontSize } = useSizeStyle(decl.location);
  let sizeStyle = getSizeStyle();

  return (
    <div
      ref={setNodeRef}
      {...listeners}
      {...attributes}
      aria-label={`func-${decl.name}-${decl.id}`}
      key={decl.id}
      className='absolute border-1 rounded-sm border-gray-200
      bg-gray-950 font-code'
      style={{ ...sizeStyle, ...transformStyle }}
    >
      <p
        className='w-full border-b border-gray-200'
        style={{ ...getFontSize(16) }}
      >
        {decl.name}
      </p>
      <div className='absolute'>{decl.body.map(displayNodes)}</div>
    </div>
  );
}

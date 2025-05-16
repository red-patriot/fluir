import { FunctionDecl, Node } from '../../models/fluir_module';
import ConstantElement from './ConstantElement';
import OpBinaryElement from './OpBinaryElement';
import OpUnaryElement from './OpUnaryElement';
import { getSizeStyle, getFontSize } from '../../hooks/useSizeStyle';
import { useDraggable, DndContext, DragEndEvent } from '@dnd-kit/core';

interface FunctionElementProps {
  decl: FunctionDecl;
  parentId?: string;
  onDragEnd?: (arg0: DragEndEvent) => void;
}

export default function FunctionElement({
  decl,
  parentId,
  onDragEnd,
}: FunctionElementProps) {
  // Constants
  const sizeStyle = getSizeStyle(decl.location);
  const fullID = parentId ? `${parentId}:${decl.id}` : `${decl.id}`;

  // Local state
  const { attributes, listeners, setNodeRef, transform } = useDraggable({
    id: fullID,
  });

  const transformStyle = transform
    ? {
        transform: `translate3d(${transform.x}px, ${transform.y}px, 0)`,
      }
    : undefined;

  // Local functions
  const displayNodes = (node: Node) => {
    if (node.discriminator === 'constant') {
      return (
        <ConstantElement
          key={node.id}
          constant={node}
          parentId={fullID}
        />
      );
    } else if (node.discriminator === 'binary') {
      return (
        <OpBinaryElement
          key={node.id}
          binary={node}
          parentId={fullID}
        />
      );
    } else if (node.discriminator === 'unary') {
      return (
        <OpUnaryElement
          key={node.id}
          unary={node}
          parentId={fullID}
        />
      );
    }
  };

  return (
    <div
      ref={setNodeRef}
      {...listeners}
      {...attributes}
      aria-label={`func-${decl.name}-${fullID}`}
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
      <div className='absolute'>
        <DndContext onDragEnd={onDragEnd}>
          {decl.body.map(displayNodes)}
        </DndContext>
      </div>
    </div>
  );
}

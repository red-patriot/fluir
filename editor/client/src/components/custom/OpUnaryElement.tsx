import { getSizeStyle, getFontSize } from '../../hooks/useSizeStyle';
import { UnaryOp } from '../../models/fluir_module';
import { useDraggable } from '@dnd-kit/core';

interface OpUnaryElementProps {
  unary: UnaryOp;
  parentId?: string;
}

export default function OpUnaryElement({
  unary,
  parentId,
}: OpUnaryElementProps) {
  // Constants
  const fullID = parentId ? `${parentId}:${unary.id}` : `${unary.id}`;

  // Local state
  const { attributes, listeners, setNodeRef, transform } = useDraggable({
    id: fullID,
  });

  const transformStyle = transform
    ? {
        transform: `translate3d(${transform.x}px, ${transform.y}px, 0)`,
      }
    : undefined;

  return (
    <div
      ref={setNodeRef}
      {...listeners}
      {...attributes}
      aria-label={`unary-${fullID}`}
      key={fullID}
      className='absolute border-2 border-orange-400
                rounded-sm
                flex justify-center font-code'
      style={{
        ...getSizeStyle(unary.location),
        ...getFontSize(unary.location),
        ...transformStyle,
      }}
    >
      {unary.op}
    </div>
  );
}

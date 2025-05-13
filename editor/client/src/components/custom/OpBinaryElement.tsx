import { getSizeStyle, getFontSize } from '../../hooks/useSizeStyle';
import { BinaryOp } from '../../models/fluir_module';
import { useDraggable } from '@dnd-kit/core';

interface OpBinaryElementProps {
  binary: BinaryOp;
  parentId?: string;
}

export default function OpBinaryElement({
  binary,
  parentId,
}: OpBinaryElementProps) {
  // Constants
  const fullID = parentId ? `${parentId}:${binary.id}` : `${binary.id}`;

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
      aria-label={`binary-${fullID}`}
      key={fullID}
      className='absolute border-2 border-yellow-300
                 rounded-sm
                 flex justify-center font-code'
      style={{
        ...getSizeStyle(binary.location),
        ...getFontSize(binary.location),
        ...transformStyle,
      }}
    >
      {binary.op}
    </div>
  );
}

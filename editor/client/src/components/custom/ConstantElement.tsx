import { getSizeStyle, getFontSize } from '../../hooks/useSizeStyle';
import { Constant } from '../../models/fluir_module';
import { useDraggable } from '@dnd-kit/core';

interface ConstantElementProps {
  constant: Constant;
  parentId?: string;
}

export default function ConstantElement({
  constant,
  parentId,
}: ConstantElementProps) {
  // Constants
  const fullID = parentId ? `${parentId}:${constant.id}` : `${constant.id}`;

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
      aria-label={`constant-${fullID}`}
      key={fullID}
      className='absolute border-2 border-purple-300
                 rounded-sm
                 flex justify-center font-code'
      style={{
        ...getSizeStyle(constant.location),
        ...getFontSize(constant.location),
        ...transformStyle,
      }}
    >
      {constant.value}
    </div>
  );
}

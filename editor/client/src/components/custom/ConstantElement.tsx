import {
  getFontSize,
  getSizeStyle,
  getLocationStyle,
} from '../../hooks/useSizeStyle';
import { Constant } from '../../models/fluir_module';
import DraggableElement from '../common/DraggableElement';
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
  const dragInfo = useDraggable({
    id: fullID,
  });

  const transformStyle = dragInfo.transform
    ? {
        transform: `translate3d(${dragInfo.transform.x}px, ${dragInfo.transform.y}px, 0)`,
      }
    : undefined;

  return (
    <div
      aria-label={`constant-${fullID}`}
      style={{
        ...getLocationStyle(constant.location),
        ...getFontSize(10),
        ...transformStyle,
      }}
      className='absolute flex flex-row items-center
    bg-purple-300 border-2 border-purple-300
    rounded-sm font-code'
    >
      <p
        key={fullID}
        className='bg-black'
        style={{
          ...getSizeStyle(constant.location),
        }}
      >
        {constant.value}
      </p>
      <DraggableElement
        dragInfo={dragInfo}
      />
      <p>#</p>
    </div>
  );
}

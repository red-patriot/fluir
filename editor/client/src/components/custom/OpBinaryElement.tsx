import {
  getFontSize,
  getSizeStyle,
  getLocationStyle,
} from '../../hooks/useSizeStyle';
import { BinaryOp } from '../../models/fluir_module';
import DraggableElement from '../common/DraggableElement';
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
      aria-label={`binary-${fullID}`}
      key={fullID}
      className='absolute flex flex-row items-center
                 border-2 border-yellow-300 bg-yellow-300
                 rounded-sm
                 font-code'
      style={{
        ...getLocationStyle(binary.location),
        ...getFontSize(binary.location),
        ...transformStyle,
      }}
    >
      <p
        className='bg-black flex justify-center items-center rounded-lg'
        style={{
          ...getSizeStyle(binary.location),
        }}
      >
        {binary.op}
      </p>
      <DraggableElement dragInfo={dragInfo} />
      <p>#</p>
    </div>
  );
}

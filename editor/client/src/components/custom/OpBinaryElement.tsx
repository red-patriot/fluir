import {
  getFontSize,
  getSizeStyle,
  getLocationStyle,
} from '../../hooks/useSizeStyle';
import { BinaryOp } from '../../models/fluir_module';
import DraggableElement from '../common/DraggableElement';
import { useDraggable } from '@dnd-kit/core';
import { InputIndicator, OutputIndicator } from '../common/InOutIndicator';

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
      className='absolute flex flex-row items-center'
      key={fullID}
      style={{
        ...getLocationStyle(binary.location),
        ...getFontSize(binary.location),
        ...transformStyle,
      }}
    >
      <InputIndicator count={2} />
      <div
        className='flex flex-row items-center
      border-[1px] border-yellow-500 bg-yellow-500
      font-code'
      >
        <p
          className='bg-black flex justify-center items-center'
          style={{
            ...getSizeStyle(binary.location),
          }}
        >
          {binary.op}
        </p>
        <DraggableElement dragInfo={dragInfo} />
      </div>
      <OutputIndicator />
    </div>
  );
}

import {
  getFontSize,
  getSizeStyle,
  getLocationStyle,
} from '../../hooks/useSizeStyle';
import { UnaryOp } from '../../models/fluir_module';
import DraggableElement from '../common/DraggableElement';
import { useDraggable } from '@dnd-kit/core';
import { InputIndicator, OutputIndicator } from '../common/InOutIndicator';

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
      aria-label={`unary-${fullID}`}
      className='absolute flex flex-row items-center'
      key={fullID}
      style={{
        ...getLocationStyle(unary.location),
        ...getFontSize(unary.location),
        ...transformStyle,
      }}
    >
      <InputIndicator />
      <div
        className='flex flex-row items-center border-[1px]
       border-orange-600 bg-orange-600 font-code'
      >
        <p
          className='bg-black flex justify-center items-center'
          style={{
            ...getSizeStyle(unary.location),
          }}
        >
          {unary.op}
        </p>
        <DraggableElement dragInfo={dragInfo} />
      </div>
      <OutputIndicator />
    </div>
  );
}

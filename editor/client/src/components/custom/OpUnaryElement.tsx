import {
  getFontSize,
  getSizeStyle,
  getLocationStyle,
} from '../../hooks/useSizeStyle';
import { UnaryOp } from '../../models/fluir_module';
import DraggableElement from '../common/DraggableElement';
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
      key={fullID}
      className='absolute border-2 border-orange-400 bg-orange-400
                rounded-sm
                flex justify-center font-code'
      style={{
        ...getLocationStyle(unary.location),
        ...getFontSize(unary.location),
        ...transformStyle,
      }}
    >
      <p
        className='bg-black flex justify-center items-center rounded-lg'
        style={{
          ...getSizeStyle(unary.location),
        }}
      >
        {unary.op}
      </p>
      <DraggableElement dragInfo={dragInfo} />
      <p>#</p>
    </div>
  );
}

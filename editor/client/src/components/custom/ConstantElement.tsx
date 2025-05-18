import { getFontSize } from '../../hooks/useSizeStyle';
import { Constant } from '../../models/fluir_module';
import { useDraggable } from '@dnd-kit/core';
import { ZOOM_SCALAR } from '../../hooks/useSizeStyle';
import { useAppSelector } from '../../store';
import { faGripVertical } from '@fortawesome/free-solid-svg-icons';
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';

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
  const { x, y, z, width, height } = constant.location;

  //Global state
  const zoom = useAppSelector((state) => state.program.zoom) * ZOOM_SCALAR;

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
      aria-label={`constant-${fullID}`}
      className='absolute flex flex-row items-center
                bg-purple-300 border-2 border-purple-300
                rounded-sm font-code'
      style={{
        left: `${x * zoom}px`,
        top: `${y * zoom}px`,
        zIndex: `${z}`,
        ...transformStyle,
        ...getFontSize(constant.location),
      }}
    >
      <p
        key={fullID}
        className='bg-black'
        style={{
          width: `${width * zoom}px`,
          height: `${height * zoom}px`,
        }}
      >
        {constant.value}
      </p>
      <div
        ref={setNodeRef}
        {...listeners}
        {...attributes}
        className='self-center h-fit border-2 border-red-400'
        style={{
          height: `${height * zoom}px`,
        }}
      >
        <FontAwesomeIcon icon={faGripVertical} />
      </div>
      <p className='self-center'>#</p>
    </div>
  );
}

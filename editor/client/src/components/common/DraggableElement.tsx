import { useDraggable } from '@dnd-kit/core';
import { faGripVertical } from '@fortawesome/free-solid-svg-icons';
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';

type DragInfo = ReturnType<typeof useDraggable>;

interface DraggableElementProps {
  dragInfo: DragInfo;
}

export default function DraggableElement({ dragInfo }: DraggableElementProps) {
  return (
    <div
      ref={dragInfo.setNodeRef}
      {...dragInfo.attributes}
      {...dragInfo.listeners}
      className='self-center cursor-move'
    >
      <FontAwesomeIcon
        className='h-full'
        icon={faGripVertical}
      />
    </div>
  );
}

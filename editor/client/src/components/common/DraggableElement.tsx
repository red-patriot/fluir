import { faGripVertical } from '@fortawesome/free-solid-svg-icons';
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';

export default function DraggableElement() {
  return (
    <div className='self-center cursor-move dragHandle__custom'>
      <FontAwesomeIcon
        className='h-full'
        icon={faGripVertical}
      />
    </div>
  );
}

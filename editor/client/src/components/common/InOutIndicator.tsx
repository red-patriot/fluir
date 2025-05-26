import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';
import { faCaretRight } from '@fortawesome/free-solid-svg-icons';

interface InOutIndicatorProps {
  count?: number;
}

export function InputIndicator({ count = 1 }: InOutIndicatorProps) {
  return (
    <div className='self-center'>
      <div
        className='flex flex-col shrink h-full pr-0.25'
        style={{
          fontSize: `${150}%`,
        }}
      >
        {Array.from({ length: count }, (_, i) => (
          <FontAwesomeIcon
            key={i}
            icon={faCaretRight}
          />
        ))}
      </div>
    </div>
  );
}

export function OutputIndicator({ count = 1 }: InOutIndicatorProps) {
  return (
    <div className='self-center'>
      <div
        className='flex flex-col shrink h-full'
        style={{
          fontSize: `${150}%`,
        }}
      >
        {Array.from({ length: count }, (_, i) => (
          <FontAwesomeIcon
            key={i}
            icon={faCaretRight}
          />
        ))}
      </div>
    </div>
  );
}

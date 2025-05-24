import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';
import { faCaretRight } from '@fortawesome/free-solid-svg-icons';

interface InOutIndicatorProps {
  count?: number;
}

export function InputIndicator({ count = 1 }: InOutIndicatorProps) {
  return (
    <div className='self-center'>
      <div
        className='flex flex-col shrink h-full overflow-hidden'
        style={{
          height: `${100 / count}%`,
          fontSize: `${150 / count}%`,
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
        className='flex flex-col shrink h-full overflow-hidden'
        style={{
          height: `${100 / count}%`,
          fontSize: `${150 / count}%`,
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

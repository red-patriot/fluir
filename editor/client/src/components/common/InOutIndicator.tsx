import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';
import { faCaretRight } from '@fortawesome/free-solid-svg-icons';
import { Handle, Position } from '@xyflow/react';

interface InOutIndicatorProps {
  fullID?: string;
  count?: number;
}

export function InputIndicator({ fullID, count = 1 }: InOutIndicatorProps) {
  return (
    <div className='self-center'>
      <div
        className='flex flex-col shrink h-full pr-0.25'
        style={{
          fontSize: `${150}%`,
        }}
      >
        {Array.from({ length: count }, (_, i) => (
          <div
            key={`output-${fullID}-${i}`}
            className='relative'
          >
            <Handle
              type='target'
              position={Position.Left}
              // The is "output" because that is what is in the source code
              // It is an input of the node, but an output to the conduit
              id={`output-${fullID}-${i}`}
              className='!w-auto !h-auto !bg-transparent !border-none !relative !transform-none'
            >
              <FontAwesomeIcon
                icon={faCaretRight}
                className='pointer-events-none text-3xl'
              />
            </Handle>
          </div>
        ))}
      </div>
    </div>
  );
}

export function OutputIndicator({ fullID, count = 1 }: InOutIndicatorProps) {
  return (
    <div className='self-center'>
      <div
        className='flex flex-col shrink h-full mr-1'
        style={{
          fontSize: `${150}%`,
        }}
      >
        {Array.from({ length: count }, (_, i) => (
          <div
            key={`output-${fullID}-${i}`}
            className='relative'
          >
            <Handle
              type='source'
              position={Position.Right}
              // The is "input" because that is what is in the source code
              // It is an output of the node, but an input to the conduit
              id={`input-${fullID}-${i}`}
              className='!w-auto !h-auto !bg-transparent !border-none !relative !transform-none'
            >
              <FontAwesomeIcon
                icon={faCaretRight}
                className='pointer-events-none text-3xl'
              />
            </Handle>
          </div>
        ))}
      </div>
    </div>
  );
}

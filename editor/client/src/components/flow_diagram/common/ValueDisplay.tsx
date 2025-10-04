import { Code } from '@radix-ui/themes';
import { useState } from 'react';

interface ValueDisplayProps extends React.PropsWithChildren {
  fullID: string;
  value: string;
  renderEdit?: (
    id: string,
    current: string,
    onDone: () => void,
  ) => React.ReactElement;
}

export function ValueDisplay({ fullID, value, renderEdit }: ValueDisplayProps) {
  const [isEditing, setIsEditing] = useState(false);

  const startEditing = () => {
    if (renderEdit) {
      setIsEditing(true);
    }
  };
  const stopEditing = () => {
    setIsEditing(false);
  };

  return (
    <Code
      color='gray'
      variant='solid'
      size='2'
      className='grow ml-0.25 overflow-hidden text-ellipsis whitespace-nowrap'
    >
      {renderEdit && isEditing ? (
        renderEdit(fullID, value, stopEditing)
      ) : (
        <span
          aria-label={`${fullID}-value-display`}
          onClick={startEditing}
        >
          {value}
        </span>
      )}
    </Code>
  );
}

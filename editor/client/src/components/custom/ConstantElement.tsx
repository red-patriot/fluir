import { useState, useEffect, useRef } from 'react';
import {
  getFontSize,
  getSizeStyle,
  getLocationStyle,
} from '../../hooks/useSizeStyle';
import { Constant } from '../../models/fluir_module';
import DraggableElement from '../common/DraggableElement';
import { useDraggable } from '@dnd-kit/core';
import EditRequest, {
  UpdateConstantEditRequest,
} from '../../models/edit_request';
import { useProgramActions } from '../common/ProgramActionsContext';
import { OutputIndicator } from '../common/InOutIndicator';

interface ConstantElementProps {
  constant: Constant;
  parentId?: string;
  onEdit?: (arg0: EditRequest) => void;
}

export default function ConstantElement({
  constant,
  parentId,
}: ConstantElementProps) {
  // Constants
  const fullID = parentId ? `${parentId}:${constant.id}` : `${constant.id}`;

  // Global State
  const { editProgram } = useProgramActions();

  // Local state
  const [isEditing, setIsEditing] = useState(false);
  const [tempText, setTempText] = useState(constant.value || ' ');
  const inputRef = useRef<HTMLInputElement>(null);
  const dragInfo = useDraggable({
    id: fullID,
  });

  const transformStyle = dragInfo.transform
    ? {
        transform: `translate3d(${dragInfo.transform.x}px, ${dragInfo.transform.y}px, 0)`,
      }
    : undefined;

  // Local functions
  const handleKeyDown = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter') {
      const request: UpdateConstantEditRequest = {
        discriminator: 'update_constant',
        target: fullID
          .toString()
          .split(':')
          .map((str) => parseInt(str)),
        value: tempText,
      };
      editProgram(request);
      setIsEditing(false);
    } else if (e.key === 'Escape') {
      setIsEditing(false);
    }
  };

  // Effects
  useEffect(() => {
    if (isEditing && inputRef.current) {
      inputRef.current.focus();
      inputRef.current.select();
    }
  }, [isEditing]);

  return (
    <div
      aria-label={`constant-${fullID}`}
      className='absolute flex flex-row items-center'
      style={{
        ...getLocationStyle(constant.location),
        ...getFontSize(10),
        ...transformStyle,
      }}
    >
      <div
        className='flex flex-row items-center space-x-0.5
      bg-purple-600 border-[1px] border-purple-600
      font-code'
      >
        {isEditing ? (
          <input
            ref={inputRef}
            type='number'
            aria-label={`constant-${fullID}-edit`}
            value={tempText}
            onChange={(e) => setTempText(e.target.value)}
            onKeyDown={handleKeyDown}
            onBlur={() => setIsEditing(false)}
            className='bg-black'
            style={{
              ...getSizeStyle(constant.location),
            }}
          />
        ) : (
          <p
            key={fullID}
            aria-label={`constant-${fullID}-view`}
            onClick={() => setIsEditing(true)}
            className='bg-black cursor-text'
            style={{
              ...getSizeStyle(constant.location),
            }}
          >
            {constant.value}
          </p>
        )}
        <DraggableElement dragInfo={dragInfo} />
      </div>
      <OutputIndicator />
    </div>
  );
}

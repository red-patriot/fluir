import { useState, useRef, useEffect } from 'react';
import { type Node, type NodeProps } from '@xyflow/react';
import { Constant } from '../../models/fluir_module';
import { getSizeStyle } from '../../hooks/useSizeStyle';
import DraggableElement from '../common/DraggableElement';
import { OutputIndicator } from '../common/InOutIndicator';
import { useProgramActions } from '../common/ProgramActionsContext';
import { UpdateConstantEditRequest } from '../../models/edit_request';

type ConstantNode = Node<{ constant: Constant; fullID: string }, 'constant'>;

export default function ConstantNode({ data }: NodeProps<ConstantNode>) {
  const { constant, fullID } = data;
  const { editProgram } = useProgramActions();

  const [isEditing, setIsEditing] = useState(false);
  const [tempText, setTempText] = useState(constant.value || ' ');
  const inputRef = useRef<HTMLInputElement>(null);

  const handleKeyDown = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter') {
      const request: UpdateConstantEditRequest = {
        discriminator: 'update_constant',
        target: fullID.split(':').map((str) => parseInt(str)),
        value: tempText,
      };
      console.log('handleKeyDown', request);
      editProgram(request);
      setIsEditing(false);
    } else if (e.key === 'Escape') {
      setIsEditing(false);
    }
  };

  useEffect(() => {
    if (isEditing && inputRef.current) {
      inputRef.current.focus();
      inputRef.current.select();
    }
  }, [isEditing]);

  return (
    <div
      className='leading-none
                flex flex-row'
    >
      <div
        className=' flex flex-row items-center
      rounded-lg
      border-1 border-purple-500 bg-purple-500 space-y-0'
      >
        <div
          className='flex justify-center items-center text-white bg-black rounded-lg font-code'
          style={{
            ...getSizeStyle(constant.location),
          }}
        >
          {isEditing ? (
            <input
              ref={inputRef}
              type='number'
              value={tempText}
              onChange={(e) => setTempText(e.target.value)}
              onKeyDown={handleKeyDown}
              onBlur={() => setIsEditing(false)}
              className='w-full h-full bg-transparent text-white outline-none'
            />
          ) : (
            <span
              onClick={() => setIsEditing(true)}
              className='cursor-text'
            >
              {constant.value ?? 'undefined'}
            </span>
          )}
        </div>
        <span className='w-[1px]' />
        <DraggableElement />
      </div>
      <OutputIndicator fullID={fullID} />
    </div>
  );
}

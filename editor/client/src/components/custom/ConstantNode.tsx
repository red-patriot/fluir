import { useState } from 'react';
import { type Node, type NodeProps } from '@xyflow/react';
import { Constant } from '../../models/fluir_module';
import { getSizeStyle } from '../../hooks/useSizeStyle';
import DraggableElement from '../common/DraggableElement';
import { OutputIndicator } from '../common/InOutIndicator';
import { useProgramActions } from '../common/ProgramActionsContext';
import { UpdateConstantEditRequest } from '../../models/edit_request';
import InputField from '../common/InputField';

type ConstantNode = Node<{ constant: Constant; fullID: string }, 'constant'>;

export default function ConstantNode({
  data: { constant, fullID },
  selected,
}: NodeProps<ConstantNode>) {
  const { editProgram } = useProgramActions();

  const [isEditing, setIsEditing] = useState(false);

  const onEnter = (value: string) => {
    const request: UpdateConstantEditRequest = {
      discriminator: 'update_constant',
      target: fullID.split(':').map((str) => parseInt(str)),
      value: value,
    };
    editProgram(request);
    setIsEditing(false);
  };
  const onCancel = () => {
    setIsEditing(false);
  };

  return (
    <div
      className={`leading-none
                flex flex-row items-center
                ${selected && 'ring-2 rounded-lg ring-white'}`}
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
            <InputField
              id={fullID}
              type='number'
              onValidateSucceed={onEnter}
              onCancel={onCancel}
              initialValue={constant.value}
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

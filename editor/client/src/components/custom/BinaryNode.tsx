import { useState } from 'react';
import type { Node, NodeProps } from '@xyflow/react';
import {
  BinaryOp,
  Operator,
  VALID_OPERATORS,
} from '../../models/fluir_module.d';
import { getSizeStyle } from '../../hooks/useSizeStyle';
import { InputIndicator, OutputIndicator } from '../common/InOutIndicator';
import DraggableElement from '../common/DraggableElement';
import InputField from '../common/InputField';
import { UpdateOperatorEditRequest } from '../../models/edit_request.d';
import { toApiID } from '../../utility/idHelpers';
import { useProgramActions } from '../common/ProgramActionsContext';

type BinaryNode = Node<{ binary: BinaryOp; fullID: string }, 'constant'>;

export default function BinaryNode({
  data: { binary, fullID },
  selected,
}: NodeProps<BinaryNode>) {
  const { editProgram } = useProgramActions();

  const [isEditing, setIsEditing] = useState(false);

  const validate = (value: string) => {
    return (VALID_OPERATORS as readonly string[]).includes(value);
  };

  const onValidateSucceed = (value: string) => {
    const request = {
      discriminator: 'update_operator',
      target: toApiID(fullID),
      value: value as Operator,
    } as UpdateOperatorEditRequest;
    editProgram(request);
    setIsEditing(false);
  };

  const onValidateFail = () => {
    // TODO: Show some error?
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
      <InputIndicator
        count={2}
        fullID={fullID}
      />
      <div
        className='leading-none
      flex flex-row border-[1px] rounded-lg
      border-blue-500 bg-blue-500 space-y-0'
      >
        <div
          className='flex flex-col justify-center items-center
                    rounded-lg text-white bg-black font-code cursor-text'
          style={{
            ...getSizeStyle(binary.location),
          }}
          onClick={() => setIsEditing(true)}
        >
          {isEditing ? (
            <InputField
              id={fullID}
              type='text'
              initialValue={binary.op}
              validate={validate}
              onValidateSucceed={onValidateSucceed}
              onValidateFail={onValidateFail}
              onCancel={onCancel}
              className='w-full h-full bg-transparent text-white text-center outline-none'
            />
          ) : (
            <span>{binary.op}</span>
          )}
        </div>
        <span className='w-[1px]' />
        <DraggableElement />
      </div>
      <OutputIndicator fullID={fullID} />
    </div>
  );
}

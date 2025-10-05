import { Flex, Code } from '@radix-ui/themes';
import {
  CreateNodeOptions,
  useDialogContext,
} from '@/components/flow_diagram/dialog/DialogContext';
import { Dialog, VisuallyHidden } from 'radix-ui';
import { slate } from '@radix-ui/colors';
import { useState } from 'react';
import { useProgramActions } from '@/components/reusable/ProgramActionsContext';
import { AddNodeEditRequest, NodeOptions } from '@/models/edit_request';
import { toApiID } from '@/utility/idHelpers';
import { LIMITS } from '@/limits';

export default function CreateNodeDialog({
  parentID,
  parentLocation,
  clickedLocation,
  where,
}: CreateNodeOptions) {
  const options = {
    Float64: 'F64',
    Int8: 'I8',
    Int16: 'I16',
    Int32: 'I32',
    Int64: 'I64',
    'Unsigned Int8': 'I8',
    'Unsigned Int16': 'I16',
    'Unsigned Int32': 'I32',
    'Unsigned Int64': 'I64',
    'Binary operator': 'BinaryOperator',
    'Unary operator': 'UnaryOperator',
  };

  const { closeDialog } = useDialogContext();
  const { editProgram } = useProgramActions();

  const [hovered, setHovered] = useState('');

  const onClick = (selection: string) => {
    const request: AddNodeEditRequest = {
      discriminator: 'add_node',
      parent: toApiID(parentID),
      new_type: options[selection as keyof typeof options] as NodeOptions,
      new_location: {
        x: clickedLocation.x - parentLocation.x,
        y: clickedLocation.y - parentLocation.y,
        z: parentLocation.z + 1,
        width: selection.endsWith('operator')
          ? LIMITS.operator.width.min
          : LIMITS.constant.width.min,
        height: selection.endsWith('operator')
          ? LIMITS.operator.height.min
          : LIMITS.constant.height.min,
      },
    };
    editProgram(request);
    closeDialog();
  };

  return (
    <Dialog.Root
      open
      modal
      onOpenChange={() => closeDialog()}
    >
      <Dialog.Trigger />
      <Dialog.Portal>
        <Dialog.Overlay className='fixed top-0 left-0 size-full bg-gray-400 opacity-30' />
        <Dialog.Content
          className='fixed'
          style={{
            top: where.y,
            left: where.x,
          }}
        >
          <VisuallyHidden.Root>
            <Dialog.Title>Create New Node</Dialog.Title>
            <Dialog.Description>Create a New Node</Dialog.Description>
          </VisuallyHidden.Root>
          <Flex
            direction='column'
            p='1'
            style={{ background: slate.slate7, borderRadius: 2 }}
          >
            {Object.keys(options).map((opt) => (
              <Code
                aria-label={`add-option-${opt}`}
                key={`add-option-${opt}`}
                variant={hovered == opt ? 'outline' : 'ghost'}
                color='blue'
                onMouseOver={() => setHovered(opt)}
                onClick={() => onClick(opt)}
                className='cursor-pointer'
              >
                {opt}
              </Code>
            ))}
          </Flex>
        </Dialog.Content>
      </Dialog.Portal>
    </Dialog.Root>
  );
}

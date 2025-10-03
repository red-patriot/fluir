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
  const options = ['Constant', 'BinaryOperator', 'UnaryOperator'];

  const { closeDialog } = useDialogContext();
  const { editProgram } = useProgramActions();

  const [hovered, setHovered] = useState('');

  const onClick = (selection: NodeOptions) => {
    const request: AddNodeEditRequest = {
      discriminator: 'add_node',
      parent: toApiID(parentID),
      new_type: selection,
      new_location: {
        x: clickedLocation.x - parentLocation.x,
        y: clickedLocation.y - parentLocation.y,
        z: parentLocation.z + 1,
        width:
          selection === 'Constant'
            ? LIMITS.constant.width.min
            : LIMITS.operator.width.min,
        height:
          selection === 'Constant'
            ? LIMITS.constant.height.min
            : LIMITS.operator.height.min,
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
            {options.map((opt) => (
              <Code
                aria-label={`add-option-${opt}`}
                key={`add-option-${opt}`}
                variant={hovered == opt ? 'outline' : 'ghost'}
                color='blue'
                onMouseOver={() => setHovered(opt)}
                onClick={() => onClick(opt as NodeOptions)}
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

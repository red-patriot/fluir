import { Flex, Code } from '@radix-ui/themes';
import {
  CreateNodeOptions,
  useDialogContext,
} from '@/components/flow_diagram/dialog/DialogContext';
import { Dialog, VisuallyHidden } from 'radix-ui';
import { slate } from '@radix-ui/colors';
import { useState } from 'react';
import { useProgramActions } from '@/components/reusable/ProgramActionsContext';
import { AddNodeEditRequest, ConstantParams, OperatorParams } from '@/models/edit_request';
import { toApiID } from '@/utility/idHelpers';
import { LIMITS } from '@/limits';
import { Completion, CompletionKind } from '@/models/intelligence_response';

interface CreateNodeDialogProps extends CreateNodeOptions {
  options: Completion[];
}

function extractWidth(kind: CompletionKind) {
  // TODO: Handle calls
  return kind === 'operator'
    ? LIMITS.operator.width.min
    : LIMITS.constant.width.min;
}

function extractHeight(kind: CompletionKind) {
  return kind === 'operator'
    ? LIMITS.operator.height.min
    : LIMITS.constant.height.min;
}

function extractParameters(completion: Completion): ConstantParams | OperatorParams {
  switch (completion.kind) {
    case 'constant':
      return {
        discriminator: 'constant',
        type: completion.short_name,
        // TODO: Add some way to handle the value
      } as ConstantParams;
    case 'operator':
      const opInfo = completion.short_name.split(' ');
      const op = opInfo[0];
      const arity = opInfo[1].includes('b') ? 'binary' : 'unary';
      return {
        discriminator: 'operator',
        arity,
        op,
      } as OperatorParams;
    case 'call':
      throw new Error('Not implemented');
  }
  throw new Error('Invalid completion kind');
}

export default function CreateNodeDialog({
                                           parentID,
                                           parentLocation,
                                           clickedLocation,
                                           where,
                                           options,
                                         }: CreateNodeDialogProps) {
  const { closeDialog } = useDialogContext();
  const { editProgram } = useProgramActions();

  const [hovered, setHovered] = useState('');

  const onClick = (selection: Completion) => {


    const request: AddNodeEditRequest = {
      discriminator: 'add_node',
      parent: toApiID(parentID),
      new_location: {
        x: clickedLocation.x - parentLocation.x,
        y: clickedLocation.y - parentLocation.y,
        z: parentLocation.z + 1,
        width: extractWidth(selection.kind),
        height: extractHeight(selection.kind),
      },
      params: extractParameters(selection),
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
        <Dialog.Overlay className="fixed top-0 left-0 size-full bg-gray-400 opacity-30" />
        <Dialog.Content
          className="fixed"
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
            direction="column"
            p="1"
            style={{ background: slate.slate7, borderRadius: 2 }}
          >
            {
              options.map((completion, i) =>
                <Code
                  aria-label={`add-option-${completion.short_name}`}
                  key={`add-option-${i}`}
                  variant={hovered == completion.short_name ? 'outline' : 'ghost'}
                  color="blue"
                  onMouseOver={() => setHovered(completion.short_name)}
                  onClick={() => onClick(completion)}
                  className="cursor-pointer"
                >
                  {completion.short_name}
                </Code>,
              )}
          </Flex>
        </Dialog.Content>
      </Dialog.Portal>
    </Dialog.Root>
  );
}

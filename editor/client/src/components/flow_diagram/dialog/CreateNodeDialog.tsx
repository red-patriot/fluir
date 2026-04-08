import { Flex, Code, TextField, Box, Separator, Theme, ScrollArea, Badge } from '@radix-ui/themes';
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
import { MagnifyingGlassIcon } from '@radix-ui/react-icons';

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
  const [searchText, setSearchText] = useState('');
  const [selectedIndex, setSelectedIndex] = useState(-1);

  const filteredOptions = options.filter((c) =>
    c.short_name.toLowerCase().startsWith(searchText.toLowerCase()),
  );

  const onSearchChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    setSearchText(e.target.value);
    setSelectedIndex(-1);
  };

  const onKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === 'ArrowDown') {
      e.preventDefault();
      setSelectedIndex((prev) =>
        Math.min(prev + 1, filteredOptions.length - 1),
      );
    } else if (e.key === 'ArrowUp') {
      e.preventDefault();
      setSelectedIndex((prev) => Math.max(prev - 1, 0));
    } else if (e.key === 'Enter' && selectedIndex >= 0) {
      e.preventDefault();
      onClick(filteredOptions[selectedIndex]);
    }
  };

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
        <Theme accentColor="blue" grayColor="gray" panelBackground="solid" radius="none" appearance="dark">

          <Dialog.Overlay className="fixed top-0 left-0 size-full bg-gray-400 opacity-30" />
          <Dialog.Content
            className="fixed w-100"
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
              style={{ background: slate.slate12, borderRadius: 2 }}
            >
              <Box>
                <TextField.Root color="gray" variant="surface" placeholder="Search…" size="3"
                                onChange={onSearchChange}
                                onKeyDown={onKeyDown}>
                  <TextField.Slot>
                    <MagnifyingGlassIcon height="16" width="16" />
                  </TextField.Slot>
                </TextField.Root>
                <Separator />
              </Box>
              <ScrollArea
                type="auto"
                scrollbars="vertical"
                style={{ maxHeight: 200 }}
              >
                {filteredOptions.map((completion, i) => (
                  <div key={`add-option-${i}`} className="p-0.5">
                    <CreateNodeDialogOption
                      aria-label={`add-option-${completion.short_name}`}
                      completion={completion}
                      onSelectOption={onClick}
                      selected={i === selectedIndex}
                      onHover={() => setSelectedIndex(i)}
                    />
                  </div>
                ))}
              </ScrollArea>
            </Flex>
          </Dialog.Content>
        </Theme>
      </Dialog.Portal>
    </Dialog.Root>
  );
}

interface CreateNodeDialogOptionProps extends React.HTMLProps<HTMLElement> {
  completion: Completion;
  onSelectOption: (selection: Completion) => void;
  selected?: boolean;
  onHover: () => void;
}

export function CreateNodeDialogOption({
                                         completion,
                                         onSelectOption,
                                         selected = false,
                                         onHover,
                                       }: CreateNodeDialogOptionProps) {
  return (
    <Badge color={selected ? 'blue' : 'gray'}
           onMouseOver={onHover}
           className="cursor-pointer w-full"
           onClick={() => onSelectOption(completion)}
    >
      <Flex direction="row" align="center" gap="2" p="2" className="w-full justify-between">
        <Code
          size="5"
          color="gray"
        >
          {completion.short_name}
        </Code>
        {/* TODO: Update the Box to contain a visual of the element to be added?*/}
        <Box />
      </Flex>
    </Badge>
  );
}

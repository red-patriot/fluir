import { Flex, TextField, Box, Separator, Theme, ScrollArea } from '@radix-ui/themes';
import {
  useDialogContext,
} from '@/components/flow_diagram/dialog/DialogContext';
import { Dialog, VisuallyHidden } from 'radix-ui';
import { slate } from '@radix-ui/colors';
import { useState } from 'react';
import { MagnifyingGlassIcon } from '@radix-ui/react-icons';

export interface OptionProps<Data> {
  data: Data;
  highlighted: boolean;
}

interface CoreDialogProps<Option> {
  where: { x: number; y: number };
  renderOption: (props: OptionProps<Option>) => React.ReactNode;
  options: Option[];
  optionToString: (option: Option) => string;
  onSelect: (option: Option) => void;
}

export default function CoreDialog<Option>({
                                             where,
                                             renderOption,
                                             options,
                                             optionToString,
                                             onSelect,
                                           }: CoreDialogProps<Option>) {
  const { closeDialog } = useDialogContext();
  const [searchText, setSearchText] = useState('');
  const [hilightedIndex, setHilightedIndex] = useState(-1);

  const filteredOptions = options.filter((o) =>
    optionToString(o).toLowerCase().startsWith(searchText.toLowerCase()),
  );

  const onSearchChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    setSearchText(e.target.value);
    setHilightedIndex(-1);
  };

  const selectOptionAndClose = (option: Option) => {
    onSelect(option);
    closeDialog();
  };

  const onKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === 'ArrowDown') {
      e.preventDefault();
      setHilightedIndex((prev) =>
        Math.min(prev + 1, filteredOptions.length - 1),
      );
    } else if (e.key === 'ArrowUp') {
      e.preventDefault();
      setHilightedIndex((prev) => Math.max(prev - 1, 0));
    } else if (e.key === 'Enter' && hilightedIndex >= 0) {
      e.preventDefault();
      selectOptionAndClose(filteredOptions[hilightedIndex]);
    }
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
              <Dialog.Title>Fluir Dialog</Dialog.Title>
              <Dialog.Description>Fluir Dialog</Dialog.Description>
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
                {filteredOptions.map((option, i) => (
                  <div key={`add-option-${i}`}
                       className="p-0.5"
                       onClick={(e) => {
                         e.stopPropagation();
                         selectOptionAndClose(option);
                       }}
                       onMouseOver={() => setHilightedIndex(i)}>
                    {
                      renderOption({
                        data: option,
                        highlighted: i === hilightedIndex,
                      })
                    }
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

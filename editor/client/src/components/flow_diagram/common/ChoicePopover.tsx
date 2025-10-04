import { Popover } from 'radix-ui';
import { Code, Flex } from '@radix-ui/themes';
import { slate } from '@radix-ui/colors';
import { useState } from 'react';

interface ChoicePopoverProps {
  id: string;
  current: string;
  choices: string[];
  onClick: (selection: string) => void;
  onDone?: () => void;
}

export function ChoicePopover({
  current,
  choices,
  onClick,
  onDone = () => {},
}: ChoicePopoverProps) {
  const [selected, setSelected] = useState(current);

  return (
    <Popover.Root open>
      <Popover.Trigger asChild>
        <span onClick={onDone}>{current}</span>
      </Popover.Trigger>
      <Popover.Content>
        <Flex
          direction='column'
          p='1'
          style={{ background: slate.slate7, borderRadius: 2 }}
        >
          {choices.map((op: string) => (
            <Code
              aria-label={`choice-${op}`}
              key={`choice-${op}`}
              className='text-center'
              variant={op == selected ? 'outline' : 'ghost'}
              color={op == selected ? 'blue' : 'gray'}
              onMouseOver={() => setSelected(op)}
              onClick={() => {
                onClick(op);
                onDone();
              }}
            >
              {op}
            </Code>
          ))}
        </Flex>
        <Popover.Arrow fill={slate.slate7} />
      </Popover.Content>
    </Popover.Root>
  );
}

export default function editWithChoicePopover(
  onClick: (selection: string) => void,
  choices: string[] = [],
) {
  return (id: string, current: string, onDone: () => void) => {
    return (
      <ChoicePopover
        id={id}
        current={current}
        choices={choices}
        onClick={onClick}
        onDone={onDone}
      />
    );
  };
}

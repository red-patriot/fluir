import { Handle, Position } from '@xyflow/react';
import { Flex } from '@radix-ui/themes';
import { gray } from '@radix-ui/colors';

interface NodeInOutProps {
  fullID?: string;
  count?: number;
}

function positionFunc(count: number) {
  return (i: number) =>
    count == 1 ? 0 : 100 * count * ((1 / (count - 1)) * i - 0.5);
}

export function NodeInput({ fullID, count = 1 }: NodeInOutProps) {
  const pos = positionFunc(count);

  return (
    <Flex
      direction='column'
      width='0'
      height='100%'
      align='start'
      gap='1'
    >
      {Array.from({ length: count }, (_, i) => {
        const position = pos(i);

        return (
          <Handle
            position={Position.Left}
            type='target'
            id={`output-${fullID}-${i}`}
            key={`output-${fullID}-${i}`}
            style={{
              translate: `-25% ${position}%`,
              backgroundColor: gray.gray10,
            }}
          ></Handle>
        );
      })}
    </Flex>
  );
}

export function NodeOutput({ fullID, count = 1 }: NodeInOutProps) {
  const pos = positionFunc(count);

  return (
    <Flex
      direction='column'
      width='0'
      height='100%'
      align='start'
      gap='1'
    >
      {Array.from({ length: count }, (_, i) => {
        const position = pos(i);

        return (
          <Handle
            position={Position.Right}
            type='source'
            id={`input-${fullID}-0`}
            style={{
              translate: `25% ${position}%`,
              backgroundColor: gray.gray10,
            }}
          ></Handle>
        );
      })}
    </Flex>
  );
}

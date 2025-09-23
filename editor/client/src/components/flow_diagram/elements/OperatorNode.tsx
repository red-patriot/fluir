import { Node, NodeProps } from '@xyflow/react';
import { cyan } from '@radix-ui/colors';
import { BinaryOp, UnaryOp } from '@/models/fluir_module';
import { Flex, Code } from '@radix-ui/themes';
import DragHandle from '../common/DragHandle';

type BinaryOperatorNode = Node<{ operator: BinaryOp; fullID: string }>;

export function BinaryOperatorNode({
  data: { operator },
}: NodeProps<BinaryOperatorNode>) {
  return (
    <Flex
      direction='row'
      height='100%'
      align='center'
      style={{ backgroundColor: cyan.cyan11 }}
    >
      <Code
        color='gray'
        variant='solid'
        size='1'
        className='ml-0.25 grow text-center'
      >
        {operator.op}
      </Code>
      <DragHandle />
    </Flex>
  );
}

type UnaryOperatorNode = Node<{ operator: UnaryOp; fullID: string }>;

export function UnaryOperatorNode({
  data: { operator },
}: NodeProps<UnaryOperatorNode>) {
  console.log(operator);
  return (
    <Flex
      direction='row'
      height='100%'
      align='center'
      style={{ backgroundColor: cyan.cyan11 }}
    >
      <Code
        color='gray'
        variant='solid'
        size='1'
        className='ml-0.25 grow text-center'
      >
        {operator.op}
      </Code>
      <DragHandle />
    </Flex>
  );
}

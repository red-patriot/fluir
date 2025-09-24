import { Node, NodeProps } from '@xyflow/react';
import { cyan } from '@radix-ui/colors';
import { BinaryOp, UnaryOp } from '@/models/fluir_module';
import { Flex, Code } from '@radix-ui/themes';
import DragHandle from '@/components/flow_diagram/common/DragHandle';
import {
  NodeInput,
  NodeOutput,
} from '@/components/flow_diagram/common/NodeInOut';

type BinaryOperatorNode = Node<{ operator: BinaryOp; fullID: string }>;

export function BinaryOperatorNode({
  data: { operator, fullID },
}: NodeProps<BinaryOperatorNode>) {
  return (
    <Flex
      direction='row'
      height='100%'
      align='center'
      style={{ backgroundColor: cyan.cyan11 }}
    >
      <NodeInput
        fullID={fullID}
        count={2}
      />
      <Code
        color='gray'
        variant='solid'
        size='1'
        className='grow text-center ml-0.25'
      >
        {operator.op}
      </Code>
      <DragHandle />
      <NodeOutput
        fullID={fullID}
        count={1}
      />
    </Flex>
  );
}

type UnaryOperatorNode = Node<{ operator: UnaryOp; fullID: string }>;

export function UnaryOperatorNode({
  data: { operator, fullID },
}: NodeProps<UnaryOperatorNode>) {
  console.log(operator);
  return (
    <Flex
      direction='row'
      height='100%'
      align='center'
      style={{ backgroundColor: cyan.cyan11 }}
    >
      <NodeInput
        fullID={fullID}
        count={1}
      />
      <Code
        color='gray'
        variant='solid'
        size='1'
        className='grow text-center ml-0.25'
      >
        {operator.op}
      </Code>
      <DragHandle />
      <NodeOutput
        fullID={fullID}
        count={1}
      />
    </Flex>
  );
}

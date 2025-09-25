import { Node, NodeProps } from '@xyflow/react';
import { cyan } from '@radix-ui/colors';
import { BinaryOp, UnaryOp } from '@/models/fluir_module';
import { Flex } from '@radix-ui/themes';
import DragHandle from '@/components/flow_diagram/common/DragHandle';
import {
  NodeInput,
  NodeOutput,
} from '@/components/flow_diagram/common/NodeInOut';
import { ValueDisplay } from '@/components/flow_diagram/common/ValueDisplay';

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
      <ValueDisplay
        fullID={fullID}
        value={operator.op}
      />
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
      <ValueDisplay
        fullID={fullID}
        value={operator.op}
      />
      <DragHandle />
      <NodeOutput
        fullID={fullID}
        count={1}
      />
    </Flex>
  );
}

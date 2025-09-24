import { Code, Flex } from '@radix-ui/themes';
import { purple } from '@radix-ui/colors';
import { Node, NodeProps } from '@xyflow/react';
import { Constant } from '@/models/fluir_module';
import DragHandle from '@/components/flow_diagram/common/DragHandle';
import { ZOOM_SCALAR } from '@/hooks/useSizeStyle';
import { NodeOutput } from '@/components/flow_diagram/common/NodeInOut';
import { HorizontalResizeHandle } from '@/components/flow_diagram/common/ResizeHandle';

type ConstantNode = Node<{ constant: Constant; fullID: string }, 'value'>;

export default function ConstantNode({
  data: { constant, fullID },
  selected,
}: NodeProps<ConstantNode>) {
  return (
    <Flex
      direction='row'
      height='100%'
      align='center'
      style={{ backgroundColor: purple.purple11 }}
    >
      <Code
        color='gray'
        variant='solid'
        size='2'
        className='ml-0.25 grow'
      >
        {constant.value}
      </Code>
      <DragHandle />
      {selected && (
        <HorizontalResizeHandle
          minWidth={12 * ZOOM_SCALAR}
          fullID={fullID}
        />
      )}
      <NodeOutput
        fullID={fullID}
        count={1}
      />
    </Flex>
  );
}

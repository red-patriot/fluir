import { Box, Code, Flex } from '@radix-ui/themes';
import { purple, slate } from '@radix-ui/colors';
import { Node, NodeResizeControl, ResizeControlVariant } from '@xyflow/react';
import { Constant } from '@/models/fluir_module';
import DragHandle from '@/components/flow_diagram/common/DragHandle';
import { ZOOM_SCALAR } from '@/hooks/useSizeStyle';

type ConstantNode = Node<{ constant: Constant; fullID: string }, 'value'>;

export default function ConstantNode({
  data: { constant, fullID },
  selected,
}: ConstantNode) {
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
        <NodeResizeControl
          minWidth={12 * ZOOM_SCALAR}
          color={slate.slate8}
          variant={ResizeControlVariant.Line}
        />
      )}
    </Flex>
  );
}

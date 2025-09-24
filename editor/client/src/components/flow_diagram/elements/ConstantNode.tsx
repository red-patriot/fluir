import { Code, Flex } from '@radix-ui/themes';
import { purple, slate } from '@radix-ui/colors';
import {
  Node,
  NodeProps,
  NodeResizeControl,
  ResizeControlVariant,
  OnResizeEnd,
  ResizeDragEvent,
  ResizeParams,
} from '@xyflow/react';
import { Constant } from '@/models/fluir_module';
import DragHandle from '@/components/flow_diagram/common/DragHandle';
import { ZOOM_SCALAR } from '@/hooks/useSizeStyle';
import { NodeOutput } from '@/components/flow_diagram/common/NodeInOut';
import { resize } from '@/components/flow_diagram/logic';
import { useProgramActions } from '@/components/common/ProgramActionsContext';

type ConstantNode = Node<{ constant: Constant; fullID: string }, 'value'>;

export default function ConstantNode({
  data: { constant, fullID },
  selected,
}: NodeProps<ConstantNode>) {
  const { editProgram } = useProgramActions();

  const doResize = resize(editProgram, fullID);

  const onFinishResize: OnResizeEnd = (
    _: ResizeDragEvent,
    params: ResizeParams,
  ) => {
    doResize(params.width / ZOOM_SCALAR, params.height / ZOOM_SCALAR);
  };

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
          onResizeEnd={onFinishResize}
        />
      )}
      <NodeOutput
        fullID={fullID}
        count={1}
      />
    </Flex>
  );
}

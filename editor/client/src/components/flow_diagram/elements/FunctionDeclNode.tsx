import {
  type Node,
  type NodeProps,
  NodeResizer,
  OnResizeEnd,
  ResizeDragEvent,
  ResizeParams,
} from '@xyflow/react';
import { FunctionDecl } from '@/models/fluir_module';
import { Box, Flex, Badge } from '@radix-ui/themes';
import { ZOOM_SCALAR } from '@/hooks/useSizeStyle';
import DragHandle from '@/components/flow_diagram/common/DragHandle';
import { useProgramActions } from '@/components/common/ProgramActionsContext';
import DeclHeader from '@/components/flow_diagram/common/DeclHeader';
import { resizeMove } from '@/components/flow_diagram/logic';

type FunctionDeclNode = Node<
  { decl: FunctionDecl; fullID: string },
  'function'
>;

export const FUNC_HEADER_HEIGHT = 0;

export default function FunctionDeclNode({
  data: { decl, fullID },
}: NodeProps<FunctionDeclNode>) {
  const { editProgram } = useProgramActions();

  const doResize = resizeMove(editProgram, fullID);

  const onFinishResize: OnResizeEnd = (
    _: ResizeDragEvent,
    params: ResizeParams,
  ) => {
    doResize(
      params.width / ZOOM_SCALAR,
      params.height / ZOOM_SCALAR,
      params.x / ZOOM_SCALAR,
      params.y / ZOOM_SCALAR,
    );
  };

  return (
    <Flex
      width='100%'
      height='100%'
      direction='column'
    >
      <Box>
        <DeclHeader
          name={decl.name}
          variant='solid'
        >
          <Flex className='grow' />
          <DragHandle />
        </DeclHeader>
        <NodeResizer
          minWidth={15 * ZOOM_SCALAR}
          minHeight={15 * ZOOM_SCALAR}
          onResizeEnd={onFinishResize}
        />
      </Box>
      <Badge
        className='grow'
        variant='outline'
      >
        <Flex className='grow ' />
      </Badge>
    </Flex>
  );
}

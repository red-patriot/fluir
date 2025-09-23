import {
  type Node,
  type NodeProps,
  NodeResizer,
  OnResizeEnd,
  ResizeDragEvent,
  ResizeParams,
} from '@xyflow/react';
import { FunctionDecl } from '@/models/fluir_module';
import { Box } from '@radix-ui/themes';
import { ZOOM_SCALAR } from '@/hooks/useSizeStyle';
import DragHandle from '@/components/flow_diagram/common/DragHandle';
import { useProgramActions } from '@/components/common/ProgramActionsContext';
import { ResizeEditRequest } from '@/models/edit_request';
import { toApiID } from '@/utility/idHelpers';
import DeclHeader from '@/components/flow_diagram/common/DeclHeader';

type FunctionDeclNode = Node<
  { decl: FunctionDecl; fullID: string },
  'function'
>;

export const FUNC_HEADER_HEIGHT = 0;

export default function FunctionDeclNode({
  data: { decl, fullID },
  width,
  height,
}: NodeProps<FunctionDeclNode>) {
  const { editProgram } = useProgramActions();

  const onFinishResize: OnResizeEnd = (
    _: ResizeDragEvent,
    params: ResizeParams,
  ) => {
    const request: ResizeEditRequest = {
      discriminator: 'resize',
      target: toApiID(fullID),
      width: params.width / ZOOM_SCALAR,
      height: params.height / ZOOM_SCALAR,
      x: params.x / ZOOM_SCALAR,
      y: params.y / ZOOM_SCALAR,
    };
    editProgram(request);
  };

  return (
    <Box
      width={`${width}`}
      height={`${height}`}
    >
      <DeclHeader
        name={decl.name}
        variant='solid'
      >
        <DragHandle />
      </DeclHeader>
      <NodeResizer
        minWidth={15 * ZOOM_SCALAR}
        minHeight={15 * ZOOM_SCALAR}
        onResizeEnd={onFinishResize}
      />
    </Box>
  );
}

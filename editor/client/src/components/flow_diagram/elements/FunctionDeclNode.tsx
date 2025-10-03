import { useReactFlow, type Node, type NodeProps } from '@xyflow/react';
import { FunctionDecl } from '@/models/fluir_module';
import { Box, Flex, Badge } from '@radix-ui/themes';
import { ZOOM_SCALAR } from '@/hooks/useSizeStyle';
import DragHandle from '@/components/flow_diagram/common/DragHandle';
import DeclHeader from '@/components/flow_diagram/common/DeclHeader';
import { XYResizeHandle } from '@/components/flow_diagram/common/ResizeHandle';
import { useMemo } from 'react';
import { gray } from '@radix-ui/colors';
import { useDialogContext } from '@/components/flow_diagram/dialog';

export type FunctionDeclNode = Node<
  { decl: FunctionDecl; fullID: string },
  'function'
>;

export const FUNC_HEADER_HEIGHT = 0;

export default function FunctionDeclNode({
  data: { decl, fullID },
}: NodeProps<FunctionDeclNode>) {
  const { screenToFlowPosition } = useReactFlow();
  const { openCreateNodeDialog } = useDialogContext();
  const minWidth = useMemo(() => {
    const rightExtents = decl.nodes.map((n) => n.location.x + n.location.width);

    return Math.max(...rightExtents, 16) * ZOOM_SCALAR;
  }, [decl.nodes]);

  const minHeight = useMemo(() => {
    const bottomExtents = decl.nodes.map(
      (n) => n.location.y + n.location.height,
    );

    return Math.max(...bottomExtents, 16) * ZOOM_SCALAR;
  }, [decl.nodes]);

  const onBodyContextMenu = (event: React.MouseEvent) => {
    const clickCoord = screenToFlowPosition({
      x: event.clientX,
      y: event.clientY,
    });
    clickCoord.x /= ZOOM_SCALAR;
    clickCoord.y /= ZOOM_SCALAR;

    openCreateNodeDialog({
      clickedLocation: clickCoord,
      parentID: fullID,
      parentLocation: decl.location,
      where: { x: event.clientX, y: event.clientY },
    });
  };

  return (
    <Flex
      width='100%'
      height='100%'
      direction='column'
      style={{ borderColor: gray.gray3, borderWidth: 1 }}
    >
      <Box>
        <DeclHeader
          name={decl.name}
          variant='solid'
        >
          <Flex className='grow' />
          <DragHandle />
        </DeclHeader>
        <XYResizeHandle
          fullID={fullID}
          minWidth={minWidth}
          minHeight={minHeight}
        />
      </Box>
      <Badge
        className='grow'
        variant='soft'
        color='gray'
        onContextMenu={onBodyContextMenu}
      >
        <Flex className='grow ' />
      </Badge>
    </Flex>
  );
}

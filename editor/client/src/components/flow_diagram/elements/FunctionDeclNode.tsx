import { type Node, type NodeProps, NodeResizer } from '@xyflow/react';
import { FunctionDecl } from '@/models/fluir_module';
import { Box, Code, Flex, Inset, Card } from '@radix-ui/themes';
import { ZOOM_SCALAR } from '@/hooks/useSizeStyle';
import DragHandle from '@/components/flow_diagram/common/DragHandle';

type FunctionDeclNode = Node<
  { decl: FunctionDecl; fullID: string },
  'function'
>;

export const FUNC_HEADER_HEIGHT = 26;

export default function FunctionDeclNode({
  data: { decl },
  width,
  height,
}: NodeProps<FunctionDeclNode>) {
  console.log(width, height, decl);

  return (
    <Box
      width={`${width}`}
      height={`${height}`}
    >
      <Flex direction='row'>
        <Card
          variant='classic'
          className='grow'
        >
          <Inset>
            <Flex direction='row'>
              <Code
                variant='soft'
                size='2'
              >
                {decl.name}
              </Code>
              <Box className='grow' />
              <DragHandle />
            </Flex>
          </Inset>
        </Card>
      </Flex>
      <NodeResizer
        minWidth={50 * ZOOM_SCALAR}
        minHeight={50 * ZOOM_SCALAR}
      />
    </Box>
  );
}

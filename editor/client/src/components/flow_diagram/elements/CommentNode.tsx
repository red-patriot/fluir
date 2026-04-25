import { Node, NodeProps } from "@xyflow/react";
import { Comment } from "@/models/fluir_module";
import { Box, Flex, Code } from "@radix-ui/themes";
import { mauve } from "@radix-ui/colors";
import ElementTag from "@/components/flow_diagram/common/ElementTag.tsx";

type CommentNode = Node<{ comment: Comment; fullId: string }>;

export default function CommentNode({
  data: { comment },
}: NodeProps<CommentNode>) {
  return (
    <Flex
      direction="column"
      height="100%"
      align="center"
      style={{ backgroundColor: mauve.mauve11 }}
    >
      <Flex direction="row" width="100%">
        <ElementTag name="//" />
        <Box className="grow" />
      </Flex>
      <Code
        className="grow w-full wrap-normal overflow-hidden text-ellipsis"
        variant="solid"
        color="gray"
        size="2"
      >
        {comment.data}
      </Code>
    </Flex>
  );
}

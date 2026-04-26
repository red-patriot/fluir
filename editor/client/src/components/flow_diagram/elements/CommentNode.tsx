import { Node, NodeProps } from "@xyflow/react";
import { Comment } from "@/models/fluir_module";
import { Box, Flex } from "@radix-ui/themes";
import { mauve } from "@radix-ui/colors";
import ElementTag from "@/components/flow_diagram/common/ElementTag.tsx";
import { useProgramActions } from "@/components/reusable/ProgramActionsContext.tsx";
import { updateComment } from "@/components/flow_diagram/logic/updateNode.ts";
import { MultilineValueDisplay } from "@/components/flow_diagram/common/ValueDisplay.tsx";
import DragHandle from "@/components/flow_diagram/common/DragHandle.tsx";
import { editWithMultilineInputField } from "@/components/flow_diagram/common/InputField.tsx";
import { XYResizeHandle } from "@/components/flow_diagram/common/ResizeHandle.tsx";
import { LIMITS } from "@/limits.ts";
import { ZOOM_SCALAR } from "@/hooks/useSizeStyle.ts";

type CommentNode = Node<{ comment: Comment; fullID: string }>;

export default function CommentNode({
  data: { comment, fullID },
}: NodeProps<CommentNode>) {
  const { editProgram } = useProgramActions();

  const updateValue = updateComment(editProgram, fullID);

  const doEdit = editWithMultilineInputField({
    validate: () => true,
    onValidateSucceed: (text) => {
      if (text != comment.data) {
        updateValue(text);
      }
    },
  });

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
        <DragHandle />
      </Flex>
      <MultilineValueDisplay
        fullID={fullID}
        value={comment.data}
        renderEdit={doEdit}
      />
      <XYResizeHandle
        fullID={fullID}
        minWidth={LIMITS.comment.width.min * ZOOM_SCALAR}
        minHeight={LIMITS.comment.height.min * ZOOM_SCALAR}
      />
    </Flex>
  );
}

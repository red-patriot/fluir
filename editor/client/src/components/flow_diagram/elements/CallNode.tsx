import { NodeProps, Node, Handle, Position } from "@xyflow/react";
import { Call } from "@/models/fluir_module";
import { Code, Flex } from "@radix-ui/themes";
import { gray, amber } from "@radix-ui/colors";
import ElementTag from "@/components/flow_diagram/common/ElementTag.tsx";
import { ValueDisplay } from "@/components/flow_diagram/common/ValueDisplay.tsx";
import DragHandle from "@/components/flow_diagram/common/DragHandle.tsx";
import { NodeOutput } from "@/components/flow_diagram/common/NodeInOut.tsx";
import { HorizontalResizeHandle } from "@/components/flow_diagram/common/ResizeHandle.tsx";
import { ZOOM_SCALAR } from "@/hooks/useSizeStyle.ts";
import { LIMITS } from "@/limits.ts";
import { useProgramActions } from "@/components/reusable/ProgramActionsContext";
import {
  renameCallArg,
  reorderCallArg,
} from "@/components/flow_diagram/logic/updateNode";
import { editWithInputField } from "@/components/flow_diagram/common/InputField";
import { validateDeclName } from "@/components/flow_diagram/logic/validateEdit.ts";
import EditRequest from "@/models/edit_request";
import { ContextMenu } from "@radix-ui/themes";
import {
  addCallArg,
  deleteCallArg,
  addCallReturn,
  deleteCallReturn,
} from "@/components/flow_diagram/logic/updateNode";
import ReorderButtons from "@/components/flow_diagram/common/ReorderButtons.tsx";

export type CallNode = Node<{
  call: Call;
  fullID: string;
}>;

export default function CallNode({
  data: { call, fullID },
  selected,
}: NodeProps<CallNode>) {
  const { editProgram } = useProgramActions();

  const addArg = addCallArg(editProgram, fullID);
  const addReturn = addCallReturn(editProgram, fullID);
  const deleteReturn = deleteCallReturn(editProgram, fullID);

  return (
    <Flex
      direction="column"
      height="100%"
      align="center"
      style={{ backgroundColor: amber.amber8 }}
      onContextMenu={(e) => e.stopPropagation()}
    >
      <ContextMenu.Root>
        <ContextMenu.Trigger>
          <Flex direction="row" className="w-full">
            <ElementTag name="fn" />
            <ValueDisplay fullID={fullID} value={call.target} />
            <DragHandle />
          </Flex>
        </ContextMenu.Trigger>
        <ContextMenu.Content>
          <ContextMenu.Item
            onSelect={() => addArg(`arg${call.arguments.length}`)}
          >
            Add Argument
          </ContextMenu.Item>
          {call.returns ? (
            <ContextMenu.Item onSelect={deleteReturn}>
              Delete Return
            </ContextMenu.Item>
          ) : (
            <ContextMenu.Item onSelect={addReturn}>Add Return</ContextMenu.Item>
          )}
        </ContextMenu.Content>
      </ContextMenu.Root>
      <Flex direction="column" className="w-full">
        {call.arguments.map((arg, index) => (
          <CallArgumentNode
            key={`${fullID}:${index}`}
            arg={arg}
            callID={fullID}
            index={index}
            editProgram={editProgram}
            maxIndex={call.arguments.length - 1}
          />
        ))}
      </Flex>
      {call.returns && <NodeOutput fullID={fullID} count={1} />}
      {selected && (
        <HorizontalResizeHandle
          fullID={fullID}
          minWidth={LIMITS.call.width.min * ZOOM_SCALAR}
        />
      )}
    </Flex>
  );
}

interface CallArgumentProps {
  arg: string;
  callID: string;
  index: number;
  editProgram: (request: EditRequest) => void;
  maxIndex: number;
}

function CallArgumentNode({
  arg,
  callID,
  index,
  editProgram,
  maxIndex,
}: CallArgumentProps) {
  const updateName = renameCallArg(editProgram, callID, index);
  const doEdit = editWithInputField({
    validate: validateDeclName,
    onValidateSucceed: updateName,
  });
  const deleteArg = deleteCallArg(editProgram, callID);
  const reorderArg = reorderCallArg(editProgram, callID);

  const isMin = index === 0;
  const isMax = index === maxIndex;

  const moveUp = () => {
    reorderArg(index, index - 1);
  };
  const moveDown = () => {
    reorderArg(index, index + 1);
  };

  return (
    <ContextMenu.Root>
      <ContextMenu.Trigger>
        <Flex
          direction="row"
          className="w-full relative"
          onContextMenu={(e) => e.stopPropagation()}
        >
          <Handle
            position={Position.Left}
            type="target"
            id={`output-${callID}-${index}`}
            style={{
              backgroundColor: gray.gray10,
              top: "50%",
              left: 0,
              right: "auto",
              transform: "translate(-50%, -50%)",
            }}
          />
          <ValueDisplay fullID={callID} value={arg} renderEdit={doEdit} />
          <ReorderButtons
            moveUp={isMin ? undefined : moveUp}
            moveDown={isMax ? undefined : moveDown}
          />
        </Flex>
      </ContextMenu.Trigger>
      <ContextMenu.Content>
        <ContextMenu.Item onSelect={() => deleteArg(index)}>
          Delete{" "}
          <Code variant="soft" color="gray">
            {arg}
          </Code>
        </ContextMenu.Item>
      </ContextMenu.Content>
    </ContextMenu.Root>
  );
}

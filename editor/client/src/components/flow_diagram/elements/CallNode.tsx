import { NodeProps, Node, Handle, Position } from '@xyflow/react';
import { Call } from '@/models/fluir_module';
import { Flex } from '@radix-ui/themes';
import { gray, sky } from '@radix-ui/colors';
import ElementTag from '@/components/flow_diagram/common/ElementTag.tsx';
import { ValueDisplay } from '@/components/flow_diagram/common/ValueDisplay.tsx';
import DragHandle from '@/components/flow_diagram/common/DragHandle.tsx';
import { NodeOutput } from '@/components/flow_diagram/common/NodeInOut.tsx';
import { HorizontalResizeHandle } from '@/components/flow_diagram/common/ResizeHandle.tsx';
import { ZOOM_SCALAR } from '@/hooks/useSizeStyle.ts';
import { LIMITS } from '@/limits.ts';

export type CallNode = Node<{
  call: Call;
  fullID: string;
}>;

export default function CallNode({
                                   data: { call, fullID },
                                   selected,
                                 }: NodeProps<CallNode>) {
  return (
    <Flex
      direction="column"
      height="100%"
      align="center"
      style={{ backgroundColor: sky.sky11 }}
    >
      <Flex direction="row" className="w-full">
        <ElementTag name="fn" />
        <ValueDisplay fullID={fullID} value={call.target} />
        <DragHandle />
      </Flex>
      <Flex direction="column" className="w-full">
        {call.arguments.map((arg, index) => (
          <CallArgumentNode arg={arg} callID={fullID} index={index} />
        ))}
      </Flex>
      {call.returns && <NodeOutput fullID={fullID} count={1} />}
      {selected && (
        <HorizontalResizeHandle fullID={fullID} minWidth={LIMITS.call.width.min * ZOOM_SCALAR} />
      )}
    </Flex>
  );
}

interface CallArgumentProps {
  arg: string;
  callID: string;
  index: number;
}

function CallArgumentNode({ arg, callID, index }: CallArgumentProps) {
  return (
    <Flex direction="row" className="w-full relative">
      <Handle
        position={Position.Left}
        type="target"
        id={`output-${callID}-${index}`}
        style={{
          backgroundColor: gray.gray10,
          top: '50%',
          left: 0,
          right: 'auto',
          transform: 'translate(-50%, -50%)',
        }}
      />
      <ValueDisplay fullID={callID} value={arg} />
    </Flex>
  );
}

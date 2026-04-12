import { FunctionParameter } from '@/models/fluir_module';
import { type Node, NodeProps } from '@xyflow/react';
import { Flex, Box } from '@radix-ui/themes';
import { slate } from '@radix-ui/colors';
import { ValueDisplay } from '@/components/flow_diagram/common/ValueDisplay.tsx';
import { NodeOutput } from '@/components/flow_diagram/common/NodeInOut.tsx';

type FunctionParameterNode = Node<{
  funcID: string;
  fullID: string;
  parameter: FunctionParameter;
}>;


export default function FunctionParameterNode({ data: { fullID, parameter } }: NodeProps<FunctionParameterNode>) {
  return (<Flex
    direction="row"
    height="100%"
    align="center"
    style={{ backgroundColor: slate.slate11 }}
  >
    <Flex
      direction="column"
      height="100%"
    >
      <Box className="grow" />
      <p className="text-[6px] font-mono align-text-bottom">
        IN
      </p>
    </Flex>
    <ValueDisplay
      fullID={fullID}
      value={parameter.name}
    />
    <NodeOutput fullID={fullID} count={1} />
  </Flex>);
}

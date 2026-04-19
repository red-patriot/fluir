import { FunctionParameter } from '@/models/fluir_module';
import { type Node, NodeProps } from '@xyflow/react';
import { Flex } from '@radix-ui/themes';
import { amber } from '@radix-ui/colors';
import { ValueDisplay } from '@/components/flow_diagram/common/ValueDisplay.tsx';
import { NodeOutput } from '@/components/flow_diagram/common/NodeInOut.tsx';
import ElementTag from '@/components/flow_diagram/common/ElementTag.tsx';

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
    style={{
      backgroundColor: amber.amber8,
    }}
  >
    <ElementTag name={parameter.flType ?? ''} />
    <ValueDisplay
      fullID={fullID}
      value={parameter.name}
    />
    <NodeOutput fullID={fullID} count={1} />
  </Flex>);
}

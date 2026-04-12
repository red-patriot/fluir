import { FunctionReturn } from '@/models/fluir_module';
import { type Node, NodeProps } from '@xyflow/react';
import { Flex, Box } from '@radix-ui/themes';
import { slate } from '@radix-ui/colors';
import { ValueDisplay } from '@/components/flow_diagram/common/ValueDisplay.tsx';
import { NodeInput, NodeOutput } from '@/components/flow_diagram/common/NodeInOut.tsx';

type FunctionReturnNode = Node<{
  funcID: string;
  fullID: string;
  return_: FunctionReturn;
}>;


export default function FunctionReturnNode({ data: { fullID, return_ } }: NodeProps<FunctionReturnNode>) {
  console.log(fullID);
  return (
    <Flex
      direction="row"
      height="100%"
      align="center"
      style={{ backgroundColor: slate.slate11 }}>
      <
        Flex
        direction="column"
        height="100%">
        <Box
          className="grow" />
        <p className="text-[6px] font-mono align-text-bottom">
          OUT
        </p>
      </Flex>
      <ValueDisplay
        fullID={fullID}
        value={return_.flType} />
      <NodeInput
        fullID={fullID}
        count={1}
      />
    </Flex>
  );
}

import { FunctionReturn } from "@/models/fluir_module";
import { type Node, NodeProps } from "@xyflow/react";
import { EdgeAnchor } from "@/components/flow_diagram/logic/resize";
import { Flex } from "@radix-ui/themes";
import { amber } from "@radix-ui/colors";
import { NodeInput } from "@/components/flow_diagram/common/NodeInOut.tsx";
import ElementTag from "@/components/flow_diagram/common/ElementTag.tsx";

type FunctionReturnNode = Node<{
  funcID: string;
  fullID: string;
  return_: FunctionReturn;
  edge: EdgeAnchor;
}>;

export default function FunctionReturnNode({
  data: { fullID, return_ },
}: NodeProps<FunctionReturnNode>) {
  return (
    <Flex
      direction="row"
      height="100%"
      align="center"
      style={{ backgroundColor: amber.amber8 }}
    >
      <ElementTag name={return_.flType ?? ""} />
      <NodeInput fullID={fullID} count={1} />
    </Flex>
  );
}

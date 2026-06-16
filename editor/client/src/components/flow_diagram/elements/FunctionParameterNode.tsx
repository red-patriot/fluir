import { FunctionParameter } from "@/models/fluir_module";
import { type Node, NodeProps } from "@xyflow/react";
import { Flex } from "@radix-ui/themes";
import { amber, gray } from "@radix-ui/colors";
import { ValueDisplay } from "@/components/flow_diagram/common/ValueDisplay.tsx";
import { NodeOutput } from "@/components/flow_diagram/common/NodeInOut.tsx";
import ElementTag from "@/components/flow_diagram/common/ElementTag.tsx";
import { useProgramActions } from "@/components/reusable/ProgramActionsContext";
import {
  updateFuncParamName,
  reorderFunctionParam,
} from "@/components/flow_diagram/logic/updateNode";
import { editWithInputField } from "@/components/flow_diagram/common/InputField";
import { validateDeclName } from "@/components/flow_diagram/logic/validateEdit.ts";
import { CaretDownIcon, CaretUpIcon } from "@radix-ui/react-icons";

type FunctionParameterNode = Node<{
  funcID: string;
  fullID: string;
  parameter: FunctionParameter;
  index: number;
  maxIndex: number;
}>;

export default function FunctionParameterNode({
  data: { funcID, fullID, parameter, index, maxIndex },
}: NodeProps<FunctionParameterNode>) {
  const { editProgram } = useProgramActions();
  const updateName = updateFuncParamName(editProgram, funcID, index);
  const reorder = reorderFunctionParam(editProgram, funcID);
  const doEdit = editWithInputField({
    validate: validateDeclName,
    onValidateSucceed: updateName,
  });
  const isMin = index === 0;
  const isMax = index === maxIndex;

  const moveUp = () => {
    if (isMin) {
      return;
    }
    reorder(index, index - 1);
  };
  const moveDown = () => {
    if (isMax) {
      return;
    }
    reorder(index, index + 1);
  };

  return (
    <Flex
      direction="row"
      height="100%"
      align="center"
      style={{
        backgroundColor: amber.amber8,
      }}
    >
      <ElementTag name={parameter.flType ?? ""} />
      <ValueDisplay
        fullID={fullID}
        value={parameter.name}
        renderEdit={doEdit}
      />
      <Flex direction="column">
        <CaretUpIcon color={isMin ? gray.gray8 : undefined} onClick={moveUp} />
        <CaretDownIcon
          color={isMax ? gray.gray8 : undefined}
          onClick={moveDown}
        />
      </Flex>
      <NodeOutput fullID={fullID} count={1} />
    </Flex>
  );
}

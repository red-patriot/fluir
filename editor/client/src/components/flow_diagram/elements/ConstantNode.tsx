import { Flex } from "@radix-ui/themes";
import { purple, pink, blue } from "@radix-ui/colors";
import { Node, NodeProps } from "@xyflow/react";
import { Constant } from "@/models/fluir_module";
import DragHandle from "@/components/flow_diagram/common/DragHandle";
import { ZOOM_SCALAR } from "@/hooks/useSizeStyle";
import { NodeOutput } from "@/components/flow_diagram/common/NodeInOut";
import { HorizontalResizeHandle } from "@/components/flow_diagram/common/ResizeHandle";
import { ValueDisplay } from "@/components/flow_diagram/common/ValueDisplay";
import { useProgramActions } from "@/components/reusable/ProgramActionsContext";
import { updateConstant } from "@/components/flow_diagram/logic/updateNode";
import { MaskOnIcon, MaskOffIcon } from "@radix-ui/react-icons";
import {
  validateF64,
  validateInt,
  validateUint,
  validateBool,
} from "@/components/flow_diagram/logic/validateEdit";
import { editWithInputField } from "@/components/flow_diagram/common/InputField";
import ElementTag from "@/components/flow_diagram/common/ElementTag";

type ConstantNode = Node<{ constant: Constant; fullID: string }, "value">;

type ConstantParams = {
  validate: (text: string) => boolean;
  color: string;
};

const FLOAT_PARAMS: ConstantParams = {
  validate: validateF64,
  color: purple.purple11,
};

const INT_PARAMS: ConstantParams = {
  validate: validateInt,
  color: pink.pink12,
};

const UINT_PARAMS: ConstantParams = {
  validate: validateUint,
  color: pink.pink9,
};

const BOOL_PARAMS: ConstantParams = {
  validate: validateBool,
  color: blue.blue11,
};

export default function ConstantNode(props: NodeProps<ConstantNode>) {
  const {
    data: { constant },
  } = props;

  if (constant.flType == "BOOL") {
    return BoolConstant(props);
  } else {
    return NumericConstant(props);
  }
}

export function NumericConstant({
  data: { constant, fullID },
  selected,
}: NodeProps<ConstantNode>) {
  const { editProgram } = useProgramActions();

  const updateValue = updateConstant(editProgram, fullID);

  const params = constant.flType?.startsWith("F")
    ? FLOAT_PARAMS
    : constant.flType?.startsWith("I")
      ? INT_PARAMS
      : UINT_PARAMS;

  const doEdit = editWithInputField({
    validate: params.validate,
    onValidateSucceed: updateValue,
  });

  return (
    <Flex
      direction="row"
      height="100%"
      align="center"
      style={{ backgroundColor: params.color }}
    >
      <ElementTag name={constant.flType ?? ""} />
      <ValueDisplay
        fullID={fullID}
        value={constant.value || ""}
        renderEdit={doEdit}
      />
      <DragHandle />
      {selected && (
        <HorizontalResizeHandle minWidth={12 * ZOOM_SCALAR} fullID={fullID} />
      )}
      <NodeOutput fullID={fullID} count={1} />
    </Flex>
  );
}

export function BoolConstant({
  data: { constant, fullID },
}: NodeProps<ConstantNode>) {
  return (
    <Flex
      direction="row"
      height="100%"
      align="center"
      style={{ backgroundColor: BOOL_PARAMS.color }}
    >
      {constant.value === "true" ? <MaskOnIcon /> : <MaskOffIcon />}
      <DragHandle />
      <NodeOutput fullID={fullID} count={1} />
    </Flex>
  );
}

import { FunctionParameter } from '@/models/fluir_module';
import { type Node, NodeProps } from '@xyflow/react';
import { Flex } from '@radix-ui/themes';
import { amber } from '@radix-ui/colors';
import { ValueDisplay } from '@/components/flow_diagram/common/ValueDisplay.tsx';
import { NodeOutput } from '@/components/flow_diagram/common/NodeInOut.tsx';
import ElementTag from '@/components/flow_diagram/common/ElementTag.tsx';
import { useProgramActions } from '@/components/reusable/ProgramActionsContext';
import { updateFuncParamName } from '@/components/flow_diagram/logic/updateNode';
import { editWithInputField } from '@/components/flow_diagram/common/InputField';
import {validateDeclName} from '@/components/flow_diagram/logic/validateEdit.ts';

type FunctionParameterNode = Node<{
  funcID: string;
  fullID: string;
  parameter: FunctionParameter;
  index: number
}>;


export default function FunctionParameterNode({ data: { funcID, fullID, parameter, index } }: NodeProps<FunctionParameterNode>) {
  const { editProgram } = useProgramActions();
  const updateName = updateFuncParamName(editProgram, funcID, index);
  const doEdit = editWithInputField({
    validate: validateDeclName,
    onValidateSucceed: updateName,
  });

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
      renderEdit={doEdit}
    />
    <NodeOutput fullID={fullID} count={1} />
  </Flex>);
}

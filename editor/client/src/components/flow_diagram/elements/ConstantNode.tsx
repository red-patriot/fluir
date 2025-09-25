import { Flex } from '@radix-ui/themes';
import { purple } from '@radix-ui/colors';
import { Node, NodeProps } from '@xyflow/react';
import { Constant } from '@/models/fluir_module';
import DragHandle from '@/components/flow_diagram/common/DragHandle';
import { ZOOM_SCALAR } from '@/hooks/useSizeStyle';
import { NodeOutput } from '@/components/flow_diagram/common/NodeInOut';
import { HorizontalResizeHandle } from '@/components/flow_diagram/common/ResizeHandle';
import { ValueDisplay } from '@/components/flow_diagram/common/ValueDisplay';
import { useProgramActions } from '@/components/common/ProgramActionsContext';
import { updateConstant } from '@/components/flow_diagram/logic/updateNode';
import { validateF64 } from '@/components/flow_diagram/logic/validateEdit';
import { editWithInputField } from '@/components/flow_diagram/common/InputField';

type ConstantNode = Node<{ constant: Constant; fullID: string }, 'value'>;

export default function ConstantNode({
  data: { constant, fullID },
  selected,
}: NodeProps<ConstantNode>) {
  const { editProgram } = useProgramActions();

  const updateValue = updateConstant(editProgram, fullID);

  const doEdit = editWithInputField({
    validate: validateF64,
    onValidateSucceed: updateValue,
  });

  return (
    <Flex
      direction='row'
      height='100%'
      align='center'
      style={{ backgroundColor: purple.purple11 }}
    >
      <ValueDisplay
        fullID={fullID}
        value={constant.value || ''}
        renderEdit={doEdit}
      />
      <DragHandle />
      {selected && (
        <HorizontalResizeHandle
          minWidth={12 * ZOOM_SCALAR}
          fullID={fullID}
        />
      )}
      <NodeOutput
        fullID={fullID}
        count={1}
      />
    </Flex>
  );
}

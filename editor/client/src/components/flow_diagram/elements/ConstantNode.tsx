import { Flex, Box } from '@radix-ui/themes';
import { purple, pink } from '@radix-ui/colors';
import { Node, NodeProps } from '@xyflow/react';
import { Constant } from '@/models/fluir_module';
import DragHandle from '@/components/flow_diagram/common/DragHandle';
import { ZOOM_SCALAR } from '@/hooks/useSizeStyle';
import { NodeOutput } from '@/components/flow_diagram/common/NodeInOut';
import { HorizontalResizeHandle } from '@/components/flow_diagram/common/ResizeHandle';
import { ValueDisplay } from '@/components/flow_diagram/common/ValueDisplay';
import { useProgramActions } from '@/components/reusable/ProgramActionsContext';
import { updateConstant } from '@/components/flow_diagram/logic/updateNode';
import {
  validateF64,
  validateInt,
  validateUint,
} from '@/components/flow_diagram/logic/validateEdit';
import { editWithInputField } from '@/components/flow_diagram/common/InputField';

type ConstantNode = Node<{ constant: Constant; fullID: string }, 'value'>;

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

export default function ConstantNode({
  data: { constant, fullID },
  selected,
}: NodeProps<ConstantNode>) {
  const { editProgram } = useProgramActions();

  const updateValue = updateConstant(editProgram, fullID);

  const params = constant.flType?.startsWith('F')
    ? FLOAT_PARAMS
    : constant.flType?.startsWith('I')
    ? INT_PARAMS
    : UINT_PARAMS;

  const doEdit = editWithInputField({
    validate: params.validate,
    onValidateSucceed: updateValue,
  });

  return (
    <Flex
      direction='row'
      height='100%'
      align='center'
      style={{ backgroundColor: params.color }}
    >
      <Flex
        direction='column'
        height='100%'
      >
        <Box className='grow' />
        <p className='text-[6px] font-mono align-text-bottom'>
          {constant.flType}
        </p>
      </Flex>
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

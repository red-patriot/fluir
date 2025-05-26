import type { Node, NodeProps } from '@xyflow/react';
import { Constant } from '../../models/fluir_module';
import { getFontSize, getSizeStyle } from '../../hooks/useSizeStyle';
import DraggableElement from '../common/DraggableElement';

type ConstantNode = Node<{ constant: Constant }, 'constant'>;

export default function ConstantNode({
  data: { constant },
}: NodeProps<ConstantNode>) {
  return (
    <div
      className='flex flex-row border-[1px] border-purple-500 bg-purple-500 space-y-0'
      style={getFontSize(constant.location)}
    >
      <div
        className='self-center text-center text-white bg-black font-code'
        style={{
          ...getSizeStyle(constant.location),
        }}
      >
        {constant.value ?? 'undefined'}
      </div>
      <DraggableElement />
    </div>
  );
}

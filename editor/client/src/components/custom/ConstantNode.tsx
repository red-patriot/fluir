import type { Node, NodeProps } from '@xyflow/react';
import { Constant } from '../../models/fluir_module';
import { getFontSize, getSizeStyle } from '../../hooks/useSizeStyle';

type ConstantNode = Node<{ constant: Constant }, 'constant'>;

export default function ConstantNode({
  data: { constant },
}: NodeProps<ConstantNode>) {
  return (
    <div className='border-[1px] border-purple-500 bg-purple-500 space-y-0'>
      <div
        className='text-white bg-black font-code'
        style={{
          ...getSizeStyle(constant.location),
          ...getFontSize(constant.location),
        }}
      >
        {constant.value ?? 'undefined'}
      </div>
    </div>
  );
}

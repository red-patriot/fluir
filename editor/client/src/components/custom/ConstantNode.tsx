import type { Node, NodeProps } from '@xyflow/react';
import { Constant } from '../../models/fluir_module';
import { getFontSize, getSizeStyle } from '../../hooks/useSizeStyle';
import DraggableElement from '../common/DraggableElement';
import { OutputIndicator } from '../common/InOutIndicator';

type ConstantNode = Node<{ constant: Constant }, 'constant'>;

export default function ConstantNode({
  data: { constant },
}: NodeProps<ConstantNode>) {
  return (
    <div
      className='leading-none
                flex flex-row'
      style={getFontSize(constant.location)}
    >
      <div
        className=' flex flex-row items-center
      rounded-xs
      border-[1px] border-purple-500 bg-purple-500 space-y-0'
      >
        <div
          className='flex justify-center items-center text-white bg-black rounded-xs font-code'
          style={{
            ...getSizeStyle(constant.location),
          }}
        >
          {constant.value ?? 'undefined'}
        </div>
        <span className='w-[1px]' />
        <DraggableElement />
      </div>
      <OutputIndicator />
    </div>
  );
}

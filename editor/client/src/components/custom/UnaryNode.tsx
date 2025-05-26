import type { Node, NodeProps } from '@xyflow/react';
import { UnaryOp } from '../../models/fluir_module';
import { getFontSize, getSizeStyle } from '../../hooks/useSizeStyle';
import DraggableElement from '../common/DraggableElement';
import { InputIndicator, OutputIndicator } from '../common/InOutIndicator';

type UnaryNode = Node<{ unary: UnaryOp }, 'constant'>;

export default function UnaryNode({ data: { unary } }: NodeProps<UnaryNode>) {
  return (
    <div
      className='leading-none
                flex flex-row'
      style={getFontSize(unary.location)}
    >
      <InputIndicator />
      <div
        className='leading-none
      flex flex-row items-center
      rounded-xs
      border-[1px] border-orange-400 bg-orange-400 space-y-0'
      >
        <div
          className='flex justify-center items-center rounded-xs text-white bg-black font-code'
          style={{
            ...getSizeStyle(unary.location),
          }}
        >
          {unary.op}
        </div>
        <span className='w-[1px]' />
        <DraggableElement />
      </div>
      <OutputIndicator />
    </div>
  );
}

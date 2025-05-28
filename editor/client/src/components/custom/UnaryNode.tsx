import type { Node, NodeProps } from '@xyflow/react';
import { UnaryOp } from '../../models/fluir_module';
import { getSizeStyle } from '../../hooks/useSizeStyle';
import DraggableElement from '../common/DraggableElement';
import { InputIndicator, OutputIndicator } from '../common/InOutIndicator';

type UnaryNode = Node<{ unary: UnaryOp; fullID: string }, 'constant'>;

export default function UnaryNode({
  data: { unary, fullID },
}: NodeProps<UnaryNode>) {
  return (
    <div
      className='leading-none
                flex flex-row'
    >
      <InputIndicator fullID={fullID} />
      <div
        className='leading-none
      flex flex-row items-center
      rounded-lg
      border-[1px] border-orange-400 bg-orange-400 space-y-0'
      >
        <div
          className='flex justify-center items-center rounded-lg text-white bg-black font-code'
          style={{
            ...getSizeStyle(unary.location),
          }}
        >
          {unary.op}
        </div>
        <span className='w-[1px]' />
        <DraggableElement />
      </div>
      <OutputIndicator fullID={fullID} />
    </div>
  );
}

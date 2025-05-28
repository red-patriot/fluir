import type { Node, NodeProps } from '@xyflow/react';
import { BinaryOp } from '../../models/fluir_module';
import { getSizeStyle } from '../../hooks/useSizeStyle';
import DraggableElement from '../common/DraggableElement';
import { InputIndicator, OutputIndicator } from '../common/InOutIndicator';

type BinaryNode = Node<{ binary: BinaryOp; fullID: string }, 'constant'>;

export default function BinaryNode({
  data: { binary, fullID },
}: NodeProps<BinaryNode>) {
  return (
    <div
      className='leading-none
                flex flex-row items-center'
    >
      <InputIndicator
        count={2}
        fullID={fullID}
      />
      <div
        className='leading-none
      flex flex-row border-[1px] rounded-lg
      border-blue-500 bg-blue-500 space-y-0'
      >
        <div
          className='flex flex-col justify-center items-center
                    rounded-lg text-white bg-black font-code'
          style={{
            ...getSizeStyle(binary.location),
          }}
        >
          {binary.op}
        </div>
        <span className='w-[1px]' />
        <DraggableElement />
      </div>
      <OutputIndicator fullID={fullID} />
    </div>
  );
}

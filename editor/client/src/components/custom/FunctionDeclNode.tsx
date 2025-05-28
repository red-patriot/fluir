import type { Node, NodeProps } from '@xyflow/react';
import { FunctionDecl } from '../../models/fluir_module';
import { getSizeStyle } from '../../hooks/useSizeStyle';
import DraggableElement from '../common/DraggableElement';

type FunctionDeclNode = Node<
  { decl: FunctionDecl; fullID: string },
  'function'
>;

export default function FunctionDeclNode({
  data: { decl },
}: NodeProps<FunctionDeclNode>) {
  return (
    <div className='rounded-b-lg border-2 border-gray-200 space-y-0'>
      <div
        className='leading-none
        rounded-t-lg
                   flex flex-row absolute
                   bottom-full left-0 font-code border-t-2 border-l-2 border-r-2 w-full'
      >
        <p>{decl.name}</p>
        <span className='grow' />
        <DraggableElement />
      </div>
      <div
        className='text-gray-600'
        style={getSizeStyle(decl.location)}
      ></div>
    </div>
  );
}

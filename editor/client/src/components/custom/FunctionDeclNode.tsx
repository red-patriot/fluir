import type { Node, NodeProps } from '@xyflow/react';
import { FunctionDecl } from '../../models/fluir_module';
import { getSizeStyle, getFontSize } from '../../hooks/useSizeStyle';

type FunctionDeclNode = Node<{ decl: FunctionDecl }, 'function'>;

export default function FunctionDeclNode({
  data: { decl },
}: NodeProps<FunctionDeclNode>) {
  const fontSize = getFontSize(14);

  return (
    <div className='border-1 border-gray-200 space-y-0'>
      <div
        className='absolute bottom-full left-0 font-code border-t-1 border-l-1 border-r-1 w-full'
        style={fontSize}
      >
        {decl.name}
      </div>
      <div
        className='text-gray-600'
        style={getSizeStyle(decl.location)}
      ></div>
    </div>
  );
}

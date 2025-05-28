import { useRef, useState, useEffect, useCallback } from 'react';
import { type Node, type NodeProps } from '@xyflow/react';
import { FunctionDecl } from '../../models/fluir_module';
import { getSizeStyle } from '../../hooks/useSizeStyle';
import DraggableElement from '../common/DraggableElement';

type FunctionDeclNode = Node<
  { decl: FunctionDecl; fullID: string },
  'function'
>;

export default function FunctionDeclNode({
  data: { decl },
  selected,
}: NodeProps<FunctionDeclNode>) {
  const headerRef = useRef<HTMLDivElement>(null);
  const [yOffset, setYOffset] = useState(0);

  useEffect(() => {
    if (headerRef.current) {
      setYOffset(-headerRef.current.offsetHeight);
    }
  }, [decl.name]);

  return (
    <div
      className={`rounded-lg border-2 border-gray-200 space-y-0
                    ${selected && 'ring-2 rounded-lg ring-white'}`}
      style={{ transform: `translateY(${yOffset}px)` }}
    >
      <div
        ref={headerRef}
        className='leading-none
                   flex flex-row font-code rounded-t-lg
                   border-b bg-slate-700 p-1 w-full'
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

import { useRef, useState, useEffect } from 'react';
import { type Node, type NodeProps } from '@xyflow/react';
import { FunctionDecl } from '../../models/fluir_module';
import { getSizeStyle, ZOOM_SCALAR } from '../../hooks/useSizeStyle';
import DraggableElement from '../common/DraggableElement';
import { ContextMenu } from 'radix-ui';
import AddNodeMenu from './AddNodeMenu';
import { useReactFlow, XYZPosition } from '@xyflow/react';
import { useProgramActions } from '../common/ProgramActionsContext';

type FunctionDeclNode = Node<
  { decl: FunctionDecl; fullID: string },
  'function'
>;

export default function FunctionDeclNode({
  data: { decl, fullID },
  selected,
}: NodeProps<FunctionDeclNode>) {
  const { editProgram } = useProgramActions();
  const { screenToFlowPosition } = useReactFlow();

  const headerRef = useRef<HTMLDivElement>(null);
  const [yOffset, setYOffset] = useState(0);
  const [addNodePosition, setAddNodePosition] = useState<
    XYZPosition | undefined
  >(undefined);

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
      <ContextMenu.Trigger
        asChild
        onContextMenu={(event: React.MouseEvent) => {
          const clickCoord = screenToFlowPosition({
            x: event.clientX,
            y: event.clientY,
          });
          const addCoord = {
            x: clickCoord.x / ZOOM_SCALAR - decl.location.x,
            y: clickCoord.y / ZOOM_SCALAR - decl.location.y,
            z: decl.location.z + 1,
          };

          setAddNodePosition(addCoord);
        }}
      >
        <div
          className='text-gray-600 cursor-copy'
          onClick={(event: React.MouseEvent) => {
            event.stopPropagation();
          }}
          style={getSizeStyle(decl.location)}
        ></div>
      </ContextMenu.Trigger>
      <AddNodeMenu
        parentID={fullID}
        editProgram={editProgram}
        addLocation={addNodePosition}
      />
    </div>
  );
}

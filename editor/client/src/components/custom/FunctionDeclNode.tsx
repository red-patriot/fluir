import { useState, useMemo } from 'react';
import { type Node, type NodeProps } from '@xyflow/react';
import { FunctionDecl } from '../../models/fluir_module';
import {
  getSizeStyle,
  HEADER_HEIGHT,
  ZOOM_SCALAR,
} from '../../hooks/useSizeStyle';
import DraggableElement from '../common/DraggableElement';
import { ContextMenu } from 'radix-ui';
import AddNodeMenu from './AddNodeMenu';
import { useReactFlow, XYZPosition } from '@xyflow/react';
import { useProgramActions } from '../common/ProgramActionsContext';

type FunctionDeclHeaderNode = Node<
  { decl: FunctionDecl; fullID: string },
  'function__header'
>;

export function FunctionDeclHeaderNode({
  data: { decl },
  selected,
}: NodeProps<FunctionDeclHeaderNode>) {
  const sizeStyle = useMemo(() => {
    let location = {
      ...decl.location,
      height: decl.location.height + HEADER_HEIGHT,
      z: decl.location.z - 1,
    };
    return getSizeStyle(location);
  }, [decl.location]);

  return (
    <div
      className={`leading-none
        rounded-lg border-2
        ${selected && 'ring-2 rounded-lg ring-white'}`}
      style={sizeStyle}
    >
      <div
        className='leading-none bg-slate-700 border-b-2
        flex flex-row font-code rounded-t-lg w-full'
      >
        <p>{decl.name}</p>
        <span className='grow' />
        <DraggableElement />
      </div>
      <div className='grow' />
    </div>
  );
}

type FunctionDeclNode = Node<
  { decl: FunctionDecl; fullID: string },
  'function'
>;

export default function FunctionDeclNode({
  data: { decl, fullID },
}: NodeProps<FunctionDeclNode>) {
  const { editProgram } = useProgramActions();
  const { screenToFlowPosition } = useReactFlow();

  const [addNodePosition, setAddNodePosition] = useState<
    XYZPosition | undefined
  >(undefined);

  return (
    <div className='rounded-lg border-gray-200'>
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
          className='cursor-copy'
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

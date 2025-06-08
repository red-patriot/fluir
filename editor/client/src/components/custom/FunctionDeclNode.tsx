import { useState } from 'react';
import { type Node, type NodeProps } from '@xyflow/react';
import { FunctionDecl } from '../../models/fluir_module';
import { getSizeStyle, ZOOM_SCALAR } from '../../hooks/useSizeStyle';
import DraggableElement from '../common/DraggableElement';
import { ContextMenu } from 'radix-ui';
import AddNodeMenu from './AddNodeMenu';
import { useReactFlow, XYZPosition } from '@xyflow/react';
import { useProgramActions } from '../common/ProgramActionsContext';
import { ResizeControl } from '../common/ResizeControl';
import { ResizeEditRequest } from '../../models/edit_request';
import { toApiID } from '../../utility/idHelpers';

export const FUNC_HEADER_HEIGHT = 4;

type FunctionDeclNode = Node<
  { decl: FunctionDecl; fullID: string },
  'function'
>;

export default function FunctionDeclNode({
  data: { decl, fullID },
  selected,
  width,
  height,
}: NodeProps<FunctionDeclNode>) {
  const { editProgram } = useProgramActions();
  const { screenToFlowPosition } = useReactFlow();

  const [addNodePosition, setAddNodePosition] = useState<
    XYZPosition | undefined
  >(undefined);

  const onFinishResize = (deltaWidth: number, deltaHeight: number) => {
    const request: ResizeEditRequest = {
      discriminator: 'resize',
      target: toApiID(fullID),
      width: decl.location.width + deltaWidth,
      height: decl.location.height + deltaHeight,
    };
    editProgram(request);
  };

  return (
    <div
      className={`rounded-lg border-2 border-gray-200
                    ${selected && 'ring-2 rounded-lg ring-white'}`}
      style={getSizeStyle(decl.location)}
    >
      <div
        className='leading-none
        flex flex-row items-center font-code rounded-t-lg
        border-b bg-slate-700 p-2 w-full'
        style={{ height: FUNC_HEADER_HEIGHT * ZOOM_SCALAR }}
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
          style={{
            height: `calc(100% - ${FUNC_HEADER_HEIGHT * ZOOM_SCALAR}px)`,
          }}
        />
      </ContextMenu.Trigger>
      <AddNodeMenu
        parentID={fullID}
        editProgram={editProgram}
        addLocation={addNodePosition}
      />
      <ResizeControl
        width={width as number}
        height={height as number}
        minWidth={15}
        minHeight={15}
        onFinishResize={onFinishResize}
      />
    </div>
  );
}

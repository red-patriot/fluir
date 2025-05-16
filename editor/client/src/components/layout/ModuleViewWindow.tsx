import { useAppDispatch, useAppSelector, actions } from '../../store';
import FunctionElement from '../custom/FunctionElement';
import { DndContext, DragEndEvent } from '@dnd-kit/core';
import EditRequest, { MoveEditRequest } from '../../models/edit_request';
import { ZOOM_SCALAR } from '../../hooks/useSizeStyle';

interface ModuleViewWindowProps {
  onEdit?: (arg0: EditRequest) => void;
}

export default function ModuleViewWindow({ onEdit }: ModuleViewWindowProps) {
  const dispatch = useAppDispatch();
  // Global State
  const module = useAppSelector((state) => state.program.module);
  const zoom = useAppSelector((state) => state.program.zoom) * ZOOM_SCALAR;

  // Local functions
  const doZoom = (event: React.WheelEvent<HTMLDivElement>) => {
    if (event.deltaY < 0) {
      dispatch(actions.zoomIn(event.deltaY / -100.0));
    } else {
      dispatch(actions.zoomOut(event.deltaY / 100.0));
    }
  };

  const handleDragDrop = (event: DragEndEvent) => {
    if (!onEdit) {
      return;
    }

    const request: MoveEditRequest = {
      discriminator: 'move',
      target: event.active.id
        .toString()
        .split(':')
        .map((str) => parseInt(str)),
      x: Math.floor(event.delta.x / zoom),
      y: Math.floor(event.delta.y / zoom),
    };
    onEdit(request);
  };

  if (!module) {
    return <p>Loading...</p>;
  }

  return (
    <div
      className='relative h-lvh w-lvw border-gray-300'
      onWheel={doZoom}
    >
      <DndContext onDragEnd={handleDragDrop}>
        {module.declarations.map((decl) => (
          <FunctionElement
            key={decl.id}
            decl={decl}
            onDragEnd={handleDragDrop}
          />
        ))}
      </DndContext>
    </div>
  );
}

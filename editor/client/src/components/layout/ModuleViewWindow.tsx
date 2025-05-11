import { useEffect } from 'react';
import { useAppDispatch, useAppSelector, actions } from '../../store';
import FunctionElement from '../custom/FunctionElement';

export default function ModuleViewWindow() {
  const dispatch = useAppDispatch();
  // Global State
  const module = useAppSelector((state) => state.program.module);

  // Local functions
  const doZoom = (event: React.WheelEvent<HTMLDivElement>) => {
    if (event.deltaY < 0) {
      dispatch(actions.zoomIn(event.deltaY / -100.0));
    } else {
      dispatch(actions.zoomOut(event.deltaY / 100.0));
    }
  };

  useEffect(() => {
    console.log(module);
    if (!module) return;
  }, []);

  if (!module) {
    return <p>Loading...</p>;
  }

  return (
    <div
      className='relative h-lvh w-lvw border-gray-300 bg-blue-950'
      onWheel={doZoom}
    >
      {module.declarations.map((decl) => (
        <FunctionElement
          key={decl.id}
          decl={decl}
        />
      ))}
    </div>
  );
}

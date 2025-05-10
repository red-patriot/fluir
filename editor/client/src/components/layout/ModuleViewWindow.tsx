import { useEffect } from 'react';
import { useAppSelector } from '../../store';
import FunctionElement from '../custom/FunctionElement';

export default function ModuleViewWindow() {
  // Global State
  const module = useAppSelector((state) => state.program.module);

  useEffect(() => {
    console.log(module);
    if (!module) return;
  }, []);

  if (!module) {
    return <p>Loading...</p>;
  }

  return (
    <div className='relative h-lvh w-lvw border-gray-300'>
      {module.declarations.map((decl) => (
        <FunctionElement decl={decl} />
      ))}
    </div>
  );
}

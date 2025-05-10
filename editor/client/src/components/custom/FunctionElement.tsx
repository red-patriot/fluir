import { FunctionDecl } from '../../models/fluir_module';

interface FunctionElementProps {
  decl: FunctionDecl;
}

export default function FunctionElement({ decl }: FunctionElementProps) {
  // TODO: Support other decl types
  const { x, y, z, width, height } = decl.location;

  return (
    <div
      key={decl.id}
      className='absolute border-2 border-gray-200 '
      style={{
        left: `${x}px`,
        top: `${y}px`,
        width: `${width}px`,
        height: `${height}px`,
        zIndex: `${z}`,
      }}
    >
      <p className='w-full border border-gray-200'>{decl.name}</p>
    </div>
  );
}

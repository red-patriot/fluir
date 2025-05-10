import { UnaryOp } from '../../models/fluir_module';

interface OpUnaryElementProps {
  unary: UnaryOp;
}

export default function OpUnaryElement({ unary }: OpUnaryElementProps) {
  const { x, y, z, width, height } = unary.location;

  return (
    <div
      key={unary.id}
      className='absolute border-2 border-orange-400'
      style={{
        left: `${x}px`,
        top: `${y}px`,
        width: `${width}px`,
        height: `${height}px`,
        zIndex: `${z}`,
      }}
    ></div>
  );
}

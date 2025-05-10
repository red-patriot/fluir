import { BinaryOp } from '../../models/fluir_module';

interface OpBinaryElementProps {
  binary: BinaryOp;
}

export default function OpBinaryElement({ binary }: OpBinaryElementProps) {
  const { x, y, z, width, height } = binary.location;

  return (
    <div
      key={binary.id}
      className='absolute border-2 border-yellow-300'
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

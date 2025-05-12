import { getSizeStyle, getFontSize } from '../../hooks/useSizeStyle';
import { BinaryOp } from '../../models/fluir_module';

interface OpBinaryElementProps {
  binary: BinaryOp;
}

export default function OpBinaryElement({ binary }: OpBinaryElementProps) {
  return (
    <div
      aria-label={`binary-${binary.id}`}
      key={binary.id}
      className='absolute border-2 border-yellow-300
                 rounded-sm
                 flex justify-center font-code'
      style={{
        ...getSizeStyle(binary.location),
        ...getFontSize(binary.location),
      }}
    >
      {binary.op}
    </div>
  );
}

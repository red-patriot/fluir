import { useSizeStyle } from '../../hooks/useSizeStyle';
import { BinaryOp } from '../../models/fluir_module';

interface OpBinaryElementProps {
  binary: BinaryOp;
}

export default function OpBinaryElement({ binary }: OpBinaryElementProps) {
  const { getSizeStyle, getFontSize } = useSizeStyle(binary.location);

  return (
    <div
      aria-label={`binary-${binary.id}`}
      key={binary.id}
      className='absolute border-2 border-yellow-300
                 rounded-sm
                 flex justify-center font-code'
      style={{ ...getSizeStyle(), ...getFontSize() }}
    >
      {binary.op}
    </div>
  );
}

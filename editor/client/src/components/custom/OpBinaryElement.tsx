import { useSizeStyle } from '../../hooks/useSizeStyle';
import { BinaryOp } from '../../models/fluir_module';

interface OpBinaryElementProps {
  binary: BinaryOp;
}

export default function OpBinaryElement({ binary }: OpBinaryElementProps) {
  const { getSizeStyle, getFontSize } = useSizeStyle(binary.location);

  return (
    <div
      key={binary.id}
      className='absolute border-2 border-yellow-300
                 rounded-lg
                 flex justify-center font-(consolas)'
      style={{ ...getSizeStyle(), ...getFontSize() }}
    >
      {binary.op}
    </div>
  );
}

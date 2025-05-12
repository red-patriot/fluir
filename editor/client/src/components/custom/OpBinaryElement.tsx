import { getSizeStyle, getFontSize } from '../../hooks/useSizeStyle';
import { BinaryOp } from '../../models/fluir_module';

interface OpBinaryElementProps {
  binary: BinaryOp;
  parentId?: string;
}

export default function OpBinaryElement({
  binary,
  parentId,
}: OpBinaryElementProps) {
  const fullID = parentId ? `${parentId}:${binary.id}` : `${binary.id}`;

  return (
    <div
      aria-label={`binary-${fullID}`}
      key={fullID}
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

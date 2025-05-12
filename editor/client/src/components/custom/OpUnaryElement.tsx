import { getSizeStyle, getFontSize } from '../../hooks/useSizeStyle';
import { UnaryOp } from '../../models/fluir_module';

interface OpUnaryElementProps {
  unary: UnaryOp;
  parentId?: string;
}

export default function OpUnaryElement({
  unary,
  parentId,
}: OpUnaryElementProps) {
  const fullID = parentId ? `${parentId}:${unary.id}` : `${unary.id}`;

  return (
    <div
      aria-label={`unary-${fullID}`}
      key={fullID}
      className='absolute border-2 border-orange-400
                rounded-sm
                flex justify-center font-code'
      style={{
        ...getSizeStyle(unary.location),
        ...getFontSize(unary.location),
      }}
    >
      {unary.op}
    </div>
  );
}

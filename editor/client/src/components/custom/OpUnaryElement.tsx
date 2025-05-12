import { getSizeStyle, getFontSize } from '../../hooks/useSizeStyle';
import { UnaryOp } from '../../models/fluir_module';

interface OpUnaryElementProps {
  unary: UnaryOp;
}

export default function OpUnaryElement({ unary }: OpUnaryElementProps) {
  return (
    <div
      aria-label={`unary-${unary.id}`}
      key={unary.id}
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

import { useSizeStyle } from '../../hooks/useSizeStyle';
import { UnaryOp } from '../../models/fluir_module';

interface OpUnaryElementProps {
  unary: UnaryOp;
}

export default function OpUnaryElement({ unary }: OpUnaryElementProps) {
  const getSizeStyle = useSizeStyle(unary.location);

  return (
    <div
      key={unary.id}
      className='absolute border-2 border-orange-400'
      style={getSizeStyle()}
    ></div>
  );
}

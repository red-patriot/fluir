import { getSizeStyle, getFontSize } from '../../hooks/useSizeStyle';
import { Constant } from '../../models/fluir_module';

interface ConstantElementProps {
  constant: Constant;
}

export default function ConstantElement({ constant }: ConstantElementProps) {
  return (
    <div
      aria-label={`constant-${constant.id}`}
      key={constant.id}
      className='absolute border-2 border-purple-300
                 rounded-sm
                 flex justify-center font-code'
      style={{
        ...getSizeStyle(constant.location),
        ...getFontSize(constant.location),
      }}
    >
      {constant.value}
    </div>
  );
}

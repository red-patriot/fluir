import { useSizeStyle } from '../../hooks/useSizeStyle';
import { Constant } from '../../models/fluir_module';

interface ConstantElementProps {
  constant: Constant;
}

export default function ConstantElement({ constant }: ConstantElementProps) {
  const { getSizeStyle, getFontSize } = useSizeStyle(constant.location);

  return (
    <div
      key={constant.id}
      className='absolute border-2 border-purple-300
                 rounded-lg
                 flex justify-center font-(consolas)'
      style={{
        ...getSizeStyle(),
        ...getFontSize(),
      }}
    >
      {constant.value}
    </div>
  );
}

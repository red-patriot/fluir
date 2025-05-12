import { getSizeStyle, getFontSize } from '../../hooks/useSizeStyle';
import { Constant } from '../../models/fluir_module';

interface ConstantElementProps {
  constant: Constant;
  parentId?: string;
}

export default function ConstantElement({
  constant,
  parentId,
}: ConstantElementProps) {
  const fullID = parentId ? `${parentId}:${constant.id}` : `${constant.id}`;

  return (
    <div
      aria-label={`constant-${fullID}`}
      key={fullID}
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

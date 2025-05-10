import { Constant } from '../../models/fluir_module';

interface ConstantElementProps {
  constant: Constant;
}

export default function ConstantElement({ constant }: ConstantElementProps) {
  const { x, y, z, width, height } = constant.location;

  return (
    <div
      key={constant.id}
      className='absolute border-2 border-purple-300'
      style={{
        left: `${x}px`,
        top: `${y}px`,
        width: `${width}px`,
        height: `${height}px`,
        zIndex: `${z}`,
      }}
    ></div>
  );
}

import { EdgeProps } from '@xyflow/react';


export default function Conduit({ sourceX, sourceY, targetX, targetY, ...props }: EdgeProps) {
  return (
    <g>
      <line
        stroke={'#3AF'}
        x1={sourceX}
        y1={sourceY}
        x2={targetX}
        y2={targetY}
        {...props} />
    </g>
  );
}

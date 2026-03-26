import { EdgeProps, Edge } from '@xyflow/react';
import { Conduit } from '@/models/fluir_module';
import { useAppSelector } from '@/store';

type ConduitEdge = Edge<{ conduit: Conduit }>;

export default function ConduitEdge({
                                      data: { conduit },
                                      sourceX,
                                      sourceY,
                                      targetX,
                                      targetY,
                                      ...props
                                    }: EdgeProps<ConduitEdge>) {
  const typeColors = useAppSelector((state) => state.ui.typeColors);
  const color =
    conduit.flType && typeColors[conduit.flType]
      ? typeColors[conduit.flType]
      : '#FF0000';
  const dashed = !(conduit.flType && typeColors[conduit.flType]);

  return (
    <g>
      <line
        stroke={color}
        strokeDasharray={dashed ? '2 2' : undefined}
        x1={sourceX}
        y1={sourceY}
        x2={targetX}
        y2={targetY}
        {...props}
      />
    </g>
  );
}

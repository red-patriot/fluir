import { useCallback, useState } from 'react';
import {
  ReactFlow,
  Background,
  BackgroundVariant,
  applyNodeChanges, NodeChange
} from '@xyflow/react';
import createNodes, { nodeTypes } from '../../hooks/createNodes';
import { useAppSelector } from '../../store';

export default function ModulePage() {
  // Global state
  const module = useAppSelector((state) => state.program.module);

  const [nodes, setNodes] = useState(
    createNodes(module ? module : { declarations: [] }),
  );

  const onNodesChange = useCallback(
    (changes: NodeChange<any>[]) =>
      setNodes((nds) => applyNodeChanges(changes, nds)),
    [],
  );

  if (!module) {
    return (
      <div className='h-lvh w-lvw flex items-center justify-center'>
        No module loaded
      </div>
    );
  }

  return (
    <div className='h-lvh w-lvw'>
      <ReactFlow
        nodes={nodes}
        nodeTypes={nodeTypes}
        onNodesChange={onNodesChange}
        fitView
        minZoom={0.1}
        maxZoom={10}
        defaultViewport={{ x: 0, y: 0, zoom: 1 }}
      >
        <Background variant={BackgroundVariant.Dots} gap={10} size={0.5}/>
      </ReactFlow>
    </div>
  );
}

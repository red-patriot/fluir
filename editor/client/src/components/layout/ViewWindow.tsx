import { useCallback, useState, useEffect } from 'react';
import {
  ReactFlow,
  Background,
  BackgroundVariant,
  applyNodeChanges,
  NodeChange,
} from '@xyflow/react';
import createNodes, { nodeTypes } from '../../hooks/createNodes';
import { useAppSelector } from '../../store';
import { useProgramActions } from '../common/ProgramActionsContext';
import { MoveEditRequest } from '../../models/edit_request';
import { ZOOM_SCALAR } from '../../hooks/useSizeStyle';

export default function ViewWindow() {
  // Global state
  const module = useAppSelector((state) => state.program.module);
  const { editProgram } = useProgramActions();

  const [nodes, setNodes] = useState(
    createNodes(module ? module : { declarations: [] }),
  );

  const onNodesChange = useCallback(
    (changes: NodeChange<any>[]) =>
      setNodes((nds) => applyNodeChanges(changes, nds)),
    [],
  );
  const onNodeDragStop = useCallback((event: React.MouseEvent, node: any) => {
    const request = {
      discriminator: 'move',
      target: (node.id as string).split(':').map((str) => parseInt(str)),
      x: Math.round(node.position.x / ZOOM_SCALAR),
      y: Math.round(node.position.y / ZOOM_SCALAR),
    } as MoveEditRequest;
    console.log('onNodeDragStop', node);
    console.log('onNodeDragStop', request);
    editProgram(request);
  }, []);

  useEffect(() => {
    setNodes(createNodes(module ? module : { declarations: [] }));
  }, [module]);

  if (!module) {
    return (
      <div className='h-lvh w-lvw flex items-center justify-center'>
        No module loaded
      </div>
    );
  }

  return (
    <div className='grow'>
      <ReactFlow
        nodes={nodes}
        nodeTypes={nodeTypes}
        onNodesChange={onNodesChange}
        onNodeDragStop={onNodeDragStop}
        fitView
        minZoom={0.1}
        maxZoom={10}
        defaultViewport={{ x: 0, y: 0, zoom: 1 }}
        snapGrid={[ZOOM_SCALAR, ZOOM_SCALAR]}
        snapToGrid
      >
        <Background
          variant={BackgroundVariant.Dots}
          gap={10}
          size={0.5}
        />
      </ReactFlow>
    </div>
  );
}

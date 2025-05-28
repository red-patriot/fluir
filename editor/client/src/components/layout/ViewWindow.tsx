import { useCallback, useState, useEffect } from 'react';
import {
  ReactFlow,
  Background,
  BackgroundVariant,
  applyNodeChanges,
  applyEdgeChanges,
  NodeChange,
  OnEdgesChange,
  DefaultEdgeOptions,
  OnConnect,
  ConnectionMode,
  useReactFlow,
  getOutgoers,
  Node,
  OnDelete,
} from '@xyflow/react';
import createNodes, { createEdges, nodeTypes } from '../../utility/createNodes';
import { useAppSelector } from '../../store';
import { useProgramActions } from '../common/ProgramActionsContext';
import {
  MoveEditRequest,
  AddConduitEditRequest,
} from '../../models/edit_request';
import { ZOOM_SCALAR } from '../../hooks/useSizeStyle';
import { toApiID } from '../../utility/idHelpers';

const defaultEdgeOptions: DefaultEdgeOptions = {
  animated: true,
};

export default function ViewWindow() {
  const module = useAppSelector((state) => state.program.module);
  const { editProgram } = useProgramActions();
  const { getNodes, getEdges } = useReactFlow();

  const [nodes, setNodes] = useState(
    createNodes(module ? module : { declarations: [] }),
  );
  const [edges, setEdges] = useState(
    createEdges(module ? module : { declarations: [] }),
  );

  const onNodesChange = useCallback(
    (changes: NodeChange<any>[]) =>
      setNodes((nds) => applyNodeChanges(changes, nds)),
    [],
  );
  const onEdgesChange: OnEdgesChange = useCallback(
    (changes) => {
      setEdges((eds) => applyEdgeChanges(changes, eds));
    },
    [setEdges],
  );

  const isValidConnection = useCallback(
    (connection: any) => {
      // we are using getNodes and getEdges helpers here
      // to make sure we create isValidConnection function only once
      const nodes = getNodes();
      const edges = getEdges();
      const target = nodes.find((node) => node.id === connection.target);
      const hasCycle = (node: Node, visited = new Set()) => {
        if (visited.has(node.id)) return false;

        visited.add(node.id);

        for (const outgoer of getOutgoers(node, nodes, edges)) {
          if (outgoer.id === connection.source) return true;
          if (hasCycle(outgoer, visited)) return true;
        }
      };

      if (!target || target.id === connection.source) return false;
      return !hasCycle(target);
    },
    [getNodes, getEdges],
  );
  const onConnect: OnConnect = useCallback(
    (connection) => {
      const request = {
        source: connection.sourceHandle,
        target: connection.targetHandle,
      } as AddConduitEditRequest;
      editProgram(request);
    },
    [setEdges],
  );

  const onNodeDragStop = useCallback(
    (_: React.MouseEvent, node: any) => {
      const request = {
        discriminator: 'move',
        target: toApiID(node.id as string),
        x: Math.round(node.position.x / ZOOM_SCALAR),
        y: Math.round(node.position.y / ZOOM_SCALAR),
      } as MoveEditRequest;
      editProgram(request);
    },
    [editProgram],
  );
  const onDelete: OnDelete = useCallback(
    (deleted) => {
      console.log('onDelete', deleted);
    },
    [editProgram],
  );

  useEffect(() => {
    setNodes(createNodes(module ? module : { declarations: [] }));
    setEdges(createEdges(module ? module : { declarations: [] }));
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
        onDelete={onDelete}
        edges={edges}
        onEdgesChange={onEdgesChange}
        connectionMode={ConnectionMode.Strict}
        onConnect={onConnect}
        isValidConnection={isValidConnection}
        defaultEdgeOptions={defaultEdgeOptions}
        fitView
        minZoom={0.1}
        maxZoom={10}
        defaultViewport={{ x: 0, y: 0, zoom: 1 }}
        snapGrid={[ZOOM_SCALAR, ZOOM_SCALAR]}
        snapToGrid
        panOnDrag={[1]}
      >
        <Background
          variant={BackgroundVariant.Dots}
          gap={50}
          size={1}
        />
      </ReactFlow>
    </div>
  );
}

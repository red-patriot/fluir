import { FunctionDecl, Node } from '../../models/fluir_module';
import ConstantElement from './ConstantElement';
import OpBinaryElement from './OpBinaryElement';
import OpUnaryElement from './OpUnaryElement';
import {
  getSizeStyle,
  getFontSize,
  getLocationStyle,
} from '../../hooks/useSizeStyle';
import { useDraggable, DndContext, DragEndEvent } from '@dnd-kit/core';
import DraggableElement from '../common/DraggableElement';

interface FunctionElementProps {
  decl: FunctionDecl;
  parentId?: string;
  onDragEnd?: (arg0: DragEndEvent) => void;
}

export default function FunctionElement({
  decl,
  parentId,
  onDragEnd,
}: FunctionElementProps) {
  // Constants
  const sizeStyle = getSizeStyle(decl.location);
  const locationStyle = getLocationStyle(decl.location);
  const fullID = parentId ? `${parentId}:${decl.id}` : `${decl.id}`;

  // Local state
  const dragInfo = useDraggable({
    id: fullID,
  });

  const transformStyle = dragInfo.transform
    ? {
        transform: `translate3d(${dragInfo.transform.x}px, ${dragInfo.transform.y}px, 0)`,
      }
    : undefined;

  // Local functions
  const displayNodes = (node: Node) => {
    if (node.discriminator === 'constant') {
      return (
        <ConstantElement
          key={node.id}
          constant={node}
          parentId={fullID}
        />
      );
    } else if (node.discriminator === 'binary') {
      return (
        <OpBinaryElement
          key={node.id}
          binary={node}
          parentId={fullID}
        />
      );
    } else if (node.discriminator === 'unary') {
      return (
        <OpUnaryElement
          key={node.id}
          unary={node}
          parentId={fullID}
        />
      );
    }
  };

  return (
    <div
      aria-label={`func-${decl.name}-${fullID}`}
      key={decl.id}
      className='absolute border-1 rounded-sm border-gray-200
      bg-black font-code'
      style={{ ...sizeStyle, ...locationStyle, ...transformStyle }}
    >
      <div
        className='flex flex-row items-center
                      border-b border-gray-200'
        style={{
          ...getFontSize(14),
        }}
      >
        <p className='grow'>{decl.name}</p>
        <DraggableElement dragInfo={dragInfo} />
      </div>
      <div className='absolute'>
        <DndContext onDragEnd={onDragEnd}>
          {decl.body.map(displayNodes)}
        </DndContext>
      </div>
    </div>
  );
}

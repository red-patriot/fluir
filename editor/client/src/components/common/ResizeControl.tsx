import { useCallback } from 'react';
import {
  NodeResizeControl,
  ResizeControlVariant,
  ResizeDragEvent,
  ResizeParams,
  OnResizeEnd,
} from '@xyflow/react';
import { ZOOM_SCALAR } from '../../hooks/useSizeStyle';
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';
import { faArrowsLeftRightToLine } from '@fortawesome/free-solid-svg-icons';

interface HorizontalResizeControlProps {
  width?: number;
  minWidth?: number;
  maxWidth?: number;
  onFinishResize?: (widthDelta: number) => void;
}

export function HorizontalResizeControl({
  width,
  minWidth,
  maxWidth,
  onFinishResize,
}: HorizontalResizeControlProps) {
  const onResizeEnd: OnResizeEnd = useCallback(
    (_: ResizeDragEvent, params: ResizeParams) => {
      if (!onFinishResize) {
        return;
      }
      const delta = Math.floor((params.width - (width || 0)) / ZOOM_SCALAR);
      onFinishResize(delta);
    },
    [onFinishResize],
  );

  return (
    <NodeResizeControl
      className='text-white flex flex-row items-center bg-transparent'
      style={{
        border: 'none',
      }}
      resizeDirection='horizontal'
      variant={ResizeControlVariant.Line}
      onResizeEnd={onResizeEnd}
      minWidth={minWidth && minWidth * ZOOM_SCALAR}
      maxWidth={maxWidth && maxWidth * ZOOM_SCALAR}
    >
      <FontAwesomeIcon
        className='h-full text-2xl'
        icon={faArrowsLeftRightToLine}
      />
    </NodeResizeControl>
  );
}

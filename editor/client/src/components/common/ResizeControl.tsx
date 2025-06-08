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
import {
  faArrowsDownToLine,
  faArrowUpRightFromSquare,
} from '@fortawesome/free-solid-svg-icons';

interface HorizontalResizeControlProps {
  width: number;
  onFinishResize: (widthDelta: number) => void;
  minWidth?: number;
  maxWidth?: number;
}

export function HorizontalResizeControl({
  width,
  minWidth,
  maxWidth,
  onFinishResize,
}: HorizontalResizeControlProps) {
  const onResizeEnd: OnResizeEnd = useCallback(
    (_: ResizeDragEvent, params: ResizeParams) => {
      const delta = Math.floor((params.width - width) / ZOOM_SCALAR);
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
        className='h-full text-2xl -translate-x-4/5 translate-y-5/4 rotate-90'
        icon={faArrowsDownToLine}
      />
    </NodeResizeControl>
  );
}

interface ResizeControlProps {
  width: number;
  height: number;
  onFinishResize: (widthDelta: number, heightDelta: number) => void;
  minWidth?: number;
  maxWidth?: number;
  minHeight?: number;
  maxHeight?: number;
}

export function ResizeControl({
  width,
  minWidth,
  maxWidth,
  height,
  minHeight,
  maxHeight,
  onFinishResize,
}: ResizeControlProps) {
  const onResizeEnd: OnResizeEnd = useCallback(
    (_: ResizeDragEvent, params: ResizeParams) => {
      const deltaW = Math.floor((params.width - width) / ZOOM_SCALAR);
      const deltaH = Math.floor((params.height - height) / ZOOM_SCALAR);

      onFinishResize(deltaW, deltaH);
    },
    [onFinishResize],
  );

  return (
    <NodeResizeControl
      className='text-white flex flex-row items-center
      -translate-x-7 -translate-y-5'
      variant={ResizeControlVariant.Handle}
      onResizeEnd={onResizeEnd}
      minWidth={minWidth && minWidth * ZOOM_SCALAR}
      maxWidth={maxWidth && maxWidth * ZOOM_SCALAR}
      minHeight={minHeight && minHeight * ZOOM_SCALAR}
      maxHeight={maxHeight && maxHeight * ZOOM_SCALAR}
    >
      <FontAwesomeIcon
        className='h-full text-2xl rotate-90'
        icon={faArrowUpRightFromSquare}
      />
    </NodeResizeControl>
  );
}

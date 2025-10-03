import {
  NodeResizeControl,
  ResizeControlVariant,
  OnResizeEnd,
  ResizeDragEvent,
  ResizeParams,
  ResizeControlProps,
} from '@xyflow/react';
import { slate } from '@radix-ui/colors';
import { ZOOM_SCALAR } from '@/hooks/useSizeStyle';
import { useProgramActions } from '@/components/reusable/ProgramActionsContext';
import { resize } from '@/components/flow_diagram/logic';
import { WidthIcon, DimensionsIcon } from '@radix-ui/react-icons';

interface ResizeHandleProps extends ResizeControlProps {
  fullID: string;
}

export function HorizontalResizeHandle({
  fullID,
  ...props
}: ResizeHandleProps) {
  const { editProgram } = useProgramActions();

  const doResize = resize(editProgram, fullID);

  const onFinishResize: OnResizeEnd = (
    _: ResizeDragEvent,
    params: ResizeParams,
  ) => {
    doResize(params.width / ZOOM_SCALAR, params.height / ZOOM_SCALAR);
  };

  return (
    <NodeResizeControl
      variant={ResizeControlVariant.Line}
      onResizeEnd={onFinishResize}
      {...props}
      style={{ border: 'none' }}
    >
      <WidthIcon
        color={slate.slate7}
        className='-translate-y-2 cursor-w-resize'
      />
    </NodeResizeControl>
  );
}

export function XYResizeHandle({ fullID, ...props }: ResizeHandleProps) {
  const { editProgram } = useProgramActions();

  const doResize = resize(editProgram, fullID);

  const onFinishResize: OnResizeEnd = (
    _: ResizeDragEvent,
    params: ResizeParams,
  ) => {
    doResize(params.width / ZOOM_SCALAR, params.height / ZOOM_SCALAR);
  };

  return (
    <NodeResizeControl
      variant={ResizeControlVariant.Handle}
      onResizeEnd={onFinishResize}
      style={{ border: 'none', background: 'none' }}
      {...props}
    >
      <DimensionsIcon
        color={slate.slate7}
        className='-translate-1 rotate-90 cursor-nwse-resize'
      />
    </NodeResizeControl>
  );
}

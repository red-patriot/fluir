import {
  NodeResizeControl,
  ResizeControlVariant,
  OnResize,
  OnResizeEnd,
  ResizeDragEvent,
  ResizeParams,
  ResizeControlProps,
  useReactFlow,
} from "@xyflow/react";
import { slate } from "@radix-ui/colors";
import { ZOOM_SCALAR } from "@/hooks/useSizeStyle";
import { useProgramActions } from "@/components/reusable/ProgramActionsContext";
import { resize, repositionReturnNodes } from "@/components/flow_diagram/logic";
import { CornerBottomRightIcon, CaretSortIcon } from "@radix-ui/react-icons";

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
      style={{ border: "none" }}
    >
      <CaretSortIcon
        color={slate.slate7}
        className="-translate-1.5 rotate-90 cursor-w-resize"
      />
    </NodeResizeControl>
  );
}

export function XYResizeHandle({ fullID, ...props }: ResizeHandleProps) {
  const { editProgram } = useProgramActions();
  const { setNodes } = useReactFlow();

  const doResize = resize(editProgram, fullID);

  const onDragResize: OnResize = (_: ResizeDragEvent, params: ResizeParams) => {
    setNodes((nds) => repositionReturnNodes(nds, fullID, params.width));
  };

  const onFinishResize: OnResizeEnd = (
    _: ResizeDragEvent,
    params: ResizeParams,
  ) => {
    doResize(params.width / ZOOM_SCALAR, params.height / ZOOM_SCALAR);
  };

  return (
    <NodeResizeControl
      variant={ResizeControlVariant.Handle}
      onResize={onDragResize}
      onResizeEnd={onFinishResize}
      style={{ border: "none", background: "none" }}
      {...props}
    >
      <CornerBottomRightIcon
        color={slate.slate7}
        className="-translate-1.5 cursor-nwse-resize"
      />
    </NodeResizeControl>
  );
}

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
import { resize, repositionEdgeNodes } from "@/components/flow_diagram/logic";
import { CornerBottomRightIcon, CaretSortIcon } from "@radix-ui/react-icons";

interface ResizeHandleProps extends ResizeControlProps {
  fullID: string;
}

/**
 * Shared resize wiring for a node: live-repositions its edge-anchored children
 * during the drag (`onResize`) and commits the new size on release
 * (`onResizeEnd`). Works for any node with children — function, loop, etc.
 */
function useResizeHandlers(fullID: string) {
  const { editProgram } = useProgramActions();
  const { setNodes } = useReactFlow();

  const doResize = resize(editProgram, fullID);

  const onResize: OnResize = (_: ResizeDragEvent, params: ResizeParams) => {
    setNodes((nds) => repositionEdgeNodes(nds, fullID, params.width));
  };

  const onResizeEnd: OnResizeEnd = (
    _: ResizeDragEvent,
    params: ResizeParams,
  ) => {
    doResize(params.width / ZOOM_SCALAR, params.height / ZOOM_SCALAR);
  };

  return { onResize, onResizeEnd };
}

export function HorizontalResizeHandle({
  fullID,
  ...props
}: ResizeHandleProps) {
  const { onResize, onResizeEnd } = useResizeHandlers(fullID);

  return (
    <NodeResizeControl
      variant={ResizeControlVariant.Line}
      onResize={onResize}
      onResizeEnd={onResizeEnd}
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
  const { onResize, onResizeEnd } = useResizeHandlers(fullID);

  return (
    <NodeResizeControl
      variant={ResizeControlVariant.Handle}
      onResize={onResize}
      onResizeEnd={onResizeEnd}
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

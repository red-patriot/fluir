import { ResizeEditRequest } from "@/models/edit_request";
import { toApiID } from "@/utility/idHelpers";
import { Node as FlowNode } from "@xyflow/react";
import { RETURN_NODE_WIDTH } from "@/utility/createNodes";
import { ZOOM_SCALAR } from "@/hooks/useSizeStyle";

/**
 * Repositions a function's return nodes to sit against its right border during
 * a live resize drag. `pixelWidth` is the Decl node's current width in pixels
 * (as reported by NodeResizeControl). Returns a new array; nodes that are not
 * return children of `funcID` are left referentially unchanged.
 */
export function repositionReturnNodes(
  nodes: FlowNode[],
  funcID: string,
  pixelWidth: number,
): FlowNode[] {
  const x = pixelWidth - RETURN_NODE_WIDTH * ZOOM_SCALAR;
  return nodes.map((node) =>
    node.parentId === funcID && node.type === "return_"
      ? { ...node, position: { ...node.position, x } }
      : node,
  );
}

export function resizeMove(
  commit: (request: ResizeEditRequest) => void,
  fullID: string,
) {
  return (width: number, height: number, x?: number, y?: number) => {
    const request: ResizeEditRequest = {
      discriminator: "resize",
      target: toApiID(fullID),
      width,
      height,
      x,
      y,
    };
    commit(request);
  };
}

export function resize(
  commit: (request: ResizeEditRequest) => void,
  fullID: string,
) {
  return (width: number, height: number) => {
    const request: ResizeEditRequest = {
      discriminator: "resize",
      target: toApiID(fullID),
      width,
      height,
    };
    commit(request);
  };
}

import { ResizeEditRequest } from "@/models/edit_request";
import { toApiID } from "@/utility/idHelpers";
import { Node as FlowNode } from "@xyflow/react";

/** Which parent border a node is anchored to. */
export type EdgeAnchor = "left" | "right";

/**
 * Repositions a function's edge-anchored child nodes to stay glued to the
 * resizing parent's border during a live resize drag. `pixelWidth` is the
 * parent node's current width in pixels (as reported by NodeResizeControl).
 *
 * A node opts in via `data.edge` ('left' | 'right'). Left-anchored nodes sit at
 * x = 0; right-anchored nodes sit at the right border (`pixelWidth - width`).
 * Returns a new array; nodes that are not edge children of `funcID` are left
 * referentially unchanged.
 */
export function repositionEdgeNodes(
  nodes: FlowNode[],
  funcID: string,
  pixelWidth: number,
): FlowNode[] {
  return nodes.map((node) => {
    if (node.parentId !== funcID) return node;
    const edge = (node.data as { edge?: EdgeAnchor }).edge;
    if (edge === "left") {
      return { ...node, position: { ...node.position, x: 0 } };
    }
    if (edge === "right") {
      return {
        ...node,
        position: { ...node.position, x: pixelWidth - (node.width ?? 0) },
      };
    }
    return node;
  });
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

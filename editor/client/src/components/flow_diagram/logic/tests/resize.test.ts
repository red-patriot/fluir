import { describe, it, expect, vi } from "vitest";
import {
  resizeMove,
  resize,
  repositionEdgeNodes,
} from "@/components/flow_diagram/logic/resize";
import { ResizeEditRequest } from "@/models/edit_request";
import { Node as FlowNode } from "@xyflow/react";

describe("resizeMove", () => {
  it("Contains the correct id", () => {
    const commitMock = vi.fn();
    const id = "0:1:2";

    const uut = resizeMove(commitMock, id);

    uut(0, 0, 0, 0);

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: "resize",
      target: [0, 1, 2],
      width: 0,
      height: 0,
      x: 0,
      y: 0,
    } as ResizeEditRequest);
  });

  it("Sends the correct values", () => {
    const commitMock = vi.fn();
    const id = "0:1:2";

    const uut = resizeMove(commitMock, id);

    uut(1, 2, 3, 4);

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: "resize",
      target: [0, 1, 2],
      width: 1,
      height: 2,
      x: 3,
      y: 4,
    } as ResizeEditRequest);
  });

  it("Optionallydoesnt send XY", () => {
    const commitMock = vi.fn();
    const id = "0:1:2";

    const uut = resizeMove(commitMock, id);

    uut(1, 2);

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: "resize",
      target: [0, 1, 2],
      width: 1,
      height: 2,
    } as ResizeEditRequest);
  });
});

describe("resize", () => {
  it("Contains the correct id", () => {
    const commitMock = vi.fn();
    const id = "0:1:2";

    const uut = resize(commitMock, id);

    uut(0, 0);

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: "resize",
      target: [0, 1, 2],
      width: 0,
      height: 0,
    } as ResizeEditRequest);
  });

  it("Sends the correct values", () => {
    const commitMock = vi.fn();
    const id = "0:1:2";

    const uut = resize(commitMock, id);

    uut(1, 2);

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: "resize",
      target: [0, 1, 2],
      width: 1,
      height: 2,
    } as ResizeEditRequest);
  });
});

describe("repositionEdgeNodes", () => {
  const parentID = "0";
  const RETURN_WIDTH = 25;
  const makeNodes = (): FlowNode[] => [
    {
      id: "0",
      type: "function",
      position: { x: 0, y: 0 },
      data: {},
    },
    {
      id: "0:1",
      type: "return_",
      parentId: parentID,
      position: { x: 0, y: 25 },
      width: RETURN_WIDTH,
      data: { edge: "right" },
    },
    {
      id: "0:2",
      type: "parameter",
      parentId: parentID,
      position: { x: 42, y: 25 },
      data: { edge: "left" },
    },
    {
      id: "0:3",
      type: "constant",
      parentId: parentID,
      position: { x: 7, y: 7 },
      data: {},
    },
  ];

  it("moves right-edge children to the resized right border", () => {
    const pixelWidth = 500;

    const result = repositionEdgeNodes(makeNodes(), parentID, pixelWidth);

    const ret = result.find((n) => n.id === "0:1")!;
    expect(ret.position.x).toBe(pixelWidth - RETURN_WIDTH);
  });

  it("pins left-edge children to x = 0 regardless of width", () => {
    const result = repositionEdgeNodes(makeNodes(), parentID, 999);

    const param = result.find((n) => n.id === "0:2")!;
    expect(param.position.x).toBe(0);
    // y preserved.
    expect(param.position.y).toBe(25);
  });

  it("leaves nodes without an edge referentially unchanged", () => {
    const input = makeNodes();

    const result = repositionEdgeNodes(input, parentID, 999);

    const constant = result.find((n) => n.id === "0:3")!;
    const func = result.find((n) => n.id === "0")!;
    expect(constant).toBe(input[3]);
    expect(func).toBe(input[0]);
  });

  it("ignores edge nodes belonging to a different function", () => {
    const input = makeNodes();

    const result = repositionEdgeNodes(input, "9", 999);

    expect(result[1]).toBe(input[1]);
    expect(result[2]).toBe(input[2]);
  });
});

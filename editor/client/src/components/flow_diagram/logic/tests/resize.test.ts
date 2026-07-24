import { describe, it, expect, vi } from "vitest";
import {
  resizeMove,
  resize,
  repositionReturnNodes,
} from "@/components/flow_diagram/logic/resize";
import { ResizeEditRequest } from "@/models/edit_request";
import { RETURN_NODE_WIDTH } from "@/utility/createNodes";
import { ZOOM_SCALAR } from "@/hooks/useSizeStyle";
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

describe("repositionReturnNodes", () => {
  const funcID = "0";
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
      parentId: funcID,
      position: { x: 0, y: 25 },
      data: {},
    },
    {
      id: "0:2",
      type: "parameter",
      parentId: funcID,
      position: { x: 0, y: 25 },
      data: {},
    },
  ];

  it("moves return_ children to the resized right border", () => {
    const pixelWidth = 100 * ZOOM_SCALAR;

    const result = repositionReturnNodes(makeNodes(), funcID, pixelWidth);

    const ret = result.find((n) => n.id === "0:1")!;
    expect(ret.position.x).toBe(pixelWidth - RETURN_NODE_WIDTH * ZOOM_SCALAR);
  });

  it("matches the x that createNodes derives from committed width", () => {
    const committedWidth = 100;
    const pixelWidth = committedWidth * ZOOM_SCALAR;

    const result = repositionReturnNodes(makeNodes(), funcID, pixelWidth);

    const ret = result.find((n) => n.id === "0:1")!;
    // createNodes computes: (decl.location.width - RETURN_NODE_WIDTH) * ZOOM_SCALAR
    expect(ret.position.x).toBe(
      (committedWidth - RETURN_NODE_WIDTH) * ZOOM_SCALAR,
    );
  });

  it("leaves non-return and unrelated nodes untouched", () => {
    const input = makeNodes();

    const result = repositionReturnNodes(input, funcID, 999);

    const param = result.find((n) => n.id === "0:2")!;
    const func = result.find((n) => n.id === "0")!;
    // Same referential objects, unchanged.
    expect(param).toBe(input[2]);
    expect(func).toBe(input[0]);
  });

  it("ignores return_ nodes belonging to a different function", () => {
    const input = makeNodes();

    const result = repositionReturnNodes(input, "9", 999);

    expect(result.find((n) => n.id === "0:1")!.position.x).toBe(0);
  });
});

import { describe, it, expect, vi } from "vitest";
import {
  updateConstant,
  updateOperator,
  renameDeclaration,
  renameCallArg,
  addCallArg,
  deleteCallArg,
  addCallReturn,
  deleteCallReturn,
} from "@/components/flow_diagram/logic/updateNode";
import {
  UpdateConstantEditRequest,
  UpdateOperatorEditRequest,
  RenameDeclarationEditRequest,
  EditCallNodeRequest,
} from "@/models/edit_request";

describe("updateConstant", () => {
  it("Contains the correct id", () => {
    const commitMock = vi.fn();
    const id = "0:1:2";

    const uut = updateConstant(commitMock, id);

    uut("");

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: "update_constant",
      target: [0, 1, 2],
      value: "",
    } as UpdateConstantEditRequest);
  });

  it("Sends the correct values", () => {
    const commitMock = vi.fn();
    const id = "0:1:2";

    const uut = updateConstant(commitMock, id);

    uut("145");

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: "update_constant",
      target: [0, 1, 2],
      value: "145",
    } as UpdateConstantEditRequest);
  });
});

describe("updateOperator", () => {
  it("Contains the correct id", () => {
    const commitMock = vi.fn();
    const id = "0:1:2";

    const uut = updateOperator(commitMock, id);

    uut("-");

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: "update_operator",
      target: [0, 1, 2],
      value: "-",
    } as UpdateOperatorEditRequest);
  });

  it("Sends the correct values", () => {
    const commitMock = vi.fn();
    const id = "0:1:2";

    const uut = updateConstant(commitMock, id);

    uut("*");

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: "update_constant",
      target: [0, 1, 2],
      value: "*",
    } as UpdateConstantEditRequest);
  });
});

describe("renameDeclaration", () => {
  it("Contains the correct id", () => {
    const commitMock = vi.fn();
    const id = "0:1:2";

    const uut = renameDeclaration(commitMock, id);

    uut("newName");

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: "rename_declaration",
      target: [0, 1, 2],
      name: "newName",
    } as RenameDeclarationEditRequest);
  });

  it("Sends the correct name", () => {
    const commitMock = vi.fn();

    const uut = renameDeclaration(commitMock, "3");

    uut("myFunc");

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: "rename_declaration",
      target: [3],
      name: "myFunc",
    } as RenameDeclarationEditRequest);
  });
});

describe("renameCallArg", () => {
  it("Contains the correct id, index, and name", () => {
    const commitMock = vi.fn();

    const uut = renameCallArg(commitMock, "0:1:2", 1);

    uut("arg1");

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: "edit_call_node",
      target: [0, 1, 2],
      command: { discriminator: "rename_arg", index: 1, name: "arg1" },
    } as EditCallNodeRequest);
  });
});

describe("addCallArg", () => {
  it("Contains the correct id and name", () => {
    const commitMock = vi.fn();

    const uut = addCallArg(commitMock, "0:1:2");

    uut("arg0");

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: "edit_call_node",
      target: [0, 1, 2],
      command: { discriminator: "add_arg", name: "arg0" },
    } as EditCallNodeRequest);
  });
});

describe("deleteCallArg", () => {
  it("Contains the correct id and index", () => {
    const commitMock = vi.fn();

    const uut = deleteCallArg(commitMock, "0:1:2");

    uut(2);

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: "edit_call_node",
      target: [0, 1, 2],
      command: { discriminator: "delete_arg", index: 2 },
    } as EditCallNodeRequest);
  });
});

describe("addCallReturn", () => {
  it("Contains the correct id and command", () => {
    const commitMock = vi.fn();

    const uut = addCallReturn(commitMock, "0:1:2");

    uut();

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: "edit_call_node",
      target: [0, 1, 2],
      command: { discriminator: "add_return" },
    } as EditCallNodeRequest);
  });
});

describe("deleteCallReturn", () => {
  it("Contains the correct id and command", () => {
    const commitMock = vi.fn();

    const uut = deleteCallReturn(commitMock, "0:1:2");

    uut();

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: "edit_call_node",
      target: [0, 1, 2],
      command: { discriminator: "delete_return" },
    } as EditCallNodeRequest);
  });
});

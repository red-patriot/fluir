import {
  UpdateConstantEditRequest,
  UpdateOperatorEditRequest,
  RenameDeclarationEditRequest,
  UpdateCommentEditRequest,
  UpdateFunctionParamRequest,
  EditCallNodeRequest,
  ReorderFunctionParamRequest,
} from "@/models/edit_request";
import { Operator } from "@/models/fluir_module";
import { toApiID } from "@/utility/idHelpers";

export function updateConstant(
  commit: (request: UpdateConstantEditRequest) => void,
  fullID: string,
) {
  return (value: string) => {
    const request: UpdateConstantEditRequest = {
      discriminator: "update_constant",
      target: toApiID(fullID),
      value,
    };
    commit(request);
  };
}

export function updateOperator(
  commit: (request: UpdateOperatorEditRequest) => void,
  fullID: string,
) {
  return (value: Operator) => {
    const request: UpdateOperatorEditRequest = {
      discriminator: "update_operator",
      target: toApiID(fullID),
      value,
    };
    commit(request);
  };
}

export function renameDeclaration(
  commit: (request: RenameDeclarationEditRequest) => void,
  fullID: string,
) {
  return (new_name: string) => {
    const request: RenameDeclarationEditRequest = {
      discriminator: "rename_declaration",
      target: toApiID(fullID),
      name: new_name,
    };
    commit(request);
  };
}

export function updateFuncParamName(
  commit: (request: UpdateFunctionParamRequest) => void,
  funcID: string,
  index: number,
) {
  return (name: string) => {
    const request: UpdateFunctionParamRequest = {
      discriminator: "update_func_param",
      target: toApiID(funcID),
      index,
      cmd: { discriminator: "name", name },
    };
    commit(request);
  };
}

export function reorderFunctionParam(
  commit: (request: ReorderFunctionParamRequest) => void,
  funcID: string,
) {
  return (source_index: number, destination_index: number) => {
    const request: ReorderFunctionParamRequest = {
      discriminator: "reorder_func_param",
      target: toApiID(funcID),
      source_index: source_index,
      destination_index: destination_index,
    };
    commit(request);
  };
}

export function renameCallArg(
  commit: (request: EditCallNodeRequest) => void,
  callID: string,
  index: number,
) {
  return (name: string) => {
    const request: EditCallNodeRequest = {
      discriminator: "edit_call_node",
      target: toApiID(callID),
      command: { discriminator: "rename_arg", index, name },
    };
    commit(request);
  };
}

export function addCallArg(
  commit: (request: EditCallNodeRequest) => void,
  callID: string,
) {
  return (name: string) => {
    const request: EditCallNodeRequest = {
      discriminator: "edit_call_node",
      target: toApiID(callID),
      command: { discriminator: "add_arg", name },
    };
    commit(request);
  };
}

export function deleteCallArg(
  commit: (request: EditCallNodeRequest) => void,
  callID: string,
) {
  return (index: number) => {
    const request: EditCallNodeRequest = {
      discriminator: "edit_call_node",
      target: toApiID(callID),
      command: { discriminator: "delete_arg", index },
    };
    commit(request);
  };
}

export function addCallReturn(
  commit: (request: EditCallNodeRequest) => void,
  callID: string,
) {
  return () => {
    const request: EditCallNodeRequest = {
      discriminator: "edit_call_node",
      target: toApiID(callID),
      command: { discriminator: "add_return" },
    };
    commit(request);
  };
}

export function deleteCallReturn(
  commit: (request: EditCallNodeRequest) => void,
  callID: string,
) {
  return () => {
    const request: EditCallNodeRequest = {
      discriminator: "edit_call_node",
      target: toApiID(callID),
      command: { discriminator: "delete_return" },
    };
    commit(request);
  };
}

export function updateComment(
  commit: (request: UpdateCommentEditRequest) => void,
  fullID: string,
) {
  return (new_data: string) => {
    console.log(fullID);
    const request: UpdateCommentEditRequest = {
      discriminator: "update_comment",
      target: toApiID(fullID),
      data: new_data,
    };
    console.log("REQUEST", request);
    commit(request);
  };
}

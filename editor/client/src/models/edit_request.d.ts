import { FlType, Location, Operator } from './fluir_module';

export type MoveEditRequest = {
  discriminator: 'move';
  target: number[];
  x: number;
  y: number;
};

export type ResizeEditRequest = {
  discriminator: 'resize';
  target: number[];
  width: number;
  height: number;
  x?: number;
  y?: number;
};

export type RenameDeclarationEditRequest = {
  discriminator: 'rename_declaration';
  target: number[];
  name: string;
};

export type UpdateConstantEditRequest = {
  discriminator: 'update_constant';
  target: number[];
  value: string;
};

export type UpdateOperatorEditRequest = {
  discriminator: 'update_operator';
  target: number[];
  value: Operator;
};

export type RenameCallArg = {
  discriminator: 'rename_arg';
  index: number;
  name: string;
};

export type EditCallNodeRequest = {
  discriminator: 'edit_call_node';
  target: number[];
  command: RenameCallArg;
};

export type AddConduitEditRequest = {
  discriminator: 'add_conduit';
  source: string; // "input-QualifiedID-index"
  target: string; // "output-QualifiedID-index"
};

export type NodeOptions = 'call' | 'constant' | 'operator';

export type ConstantParams = {
  discriminator: 'constant';
  type: 'F64' | 'I8' | 'I16' | 'I32' | 'I64' | 'U8' | 'U16' | 'U32' | 'U64';
  value: string | undefined;
};

export type OperatorParams = {
  discriminator: 'operator';
  arity: 'binary' | 'unary';
  op: string | undefined;
};

export type CallParams = {
  discriminator: 'call';
  target: string;
};

export type AddNodeEditRequest = {
  discriminator: 'add_node';
  parent: number[];
  new_location: Location;
  params: ConstantParams | OperatorParams | CallParams;
};

export type DeclParameterParams = {
  discriminator: 'parameter';
  name: string;
};

export type DeclReturnParams = {
  discriminator: 'return';
};

export type AddDeclInterfaceEditRequest = {
  discriminator: 'add_decl_interface';
  parent: number[];
  flType: FlType;
  params: DeclParameterParams | DeclReturnParams;
};

export type CreateFunctionParams = {
  discriminator: 'function';
  name?: string;
};

export type AddDeclEditRequest = {
  discriminator: 'add_decl';
  new_location: Location;
  params: CreateFunctionParams;
};

export type UpdateFuncParamType = {
  discriminator: 'type';
  flType: FlType;
};

export type UpdateFuncParamName = {
  discriminator: 'name';
  name: string;
}

export type UpdateFunctionParamRequest = {
  discriminator: 'update_func_param';
  target: number[];
  index: number;
  cmd: UpdateFuncParamType | UpdateFuncParamName;
};

export type UpdateFunctionReturnRequest = {
  discriminator: 'update_func_return';
  target: number[];
  type: FlType;
};

export type RemoveItemEditRequest = {
  target: number[];
};

export type AddCommentEditRequest = {
  discriminator: 'add_comment';
  parent: number[];
  new_location: Location;
  data: string;
};

export type UpdateCommentEditRequest = {
  discriminator: 'update_comment';
  target: number[];
  data: string;
};

type EditRequest =
  | MoveEditRequest
  | ResizeEditRequest
  | RenameDeclarationEditRequest
  | UpdateConstantEditRequest
  | UpdateCommentEditRequest
  | UpdateOperatorEditRequest
  | AddCommentEditRequest
  | AddConduitEditRequest
  | AddNodeEditRequest
  | AddDeclEditRequest
  | AddDeclInterfaceEditRequest
  | RemoveItemEditRequest
  | UpdateFunctionParamRequest
  | UpdateFunctionReturnRequest
  | EditCallNodeRequest;

export default EditRequest;

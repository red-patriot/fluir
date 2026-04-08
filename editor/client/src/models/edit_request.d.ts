import { Location, Operator } from './fluir_module';

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

export type AddConduitEditRequest = {
  discriminator: 'add_conduit';
  source: string; // "input-QualifiedID-index"
  target: string; // "output-QualifiedID-index"
};

export type NodeOptions =
  'call'
  | 'constant'
  | 'operator';

export type ConstantParams = {
  discriminator: 'constant';
  type: 'F64' | 'I8' | 'I16' | 'I32' | 'I64' | 'U8' | 'U16' | 'U32' | 'U64';
  value: string | undefined;
}

export type OperatorParams = {
  discriminator: 'operator';
  arity: 'binary' | 'unary';
  op: string | undefined;
}

export type AddNodeEditRequest = {
  discriminator: 'add_node';
  parent: number[];
  new_location: Location;
  params: ConstantParams | OperatorParams;
};

export type CreateFunctionParams = {
  discriminator: 'function';
  name?: string;
};

export type AddDeclEditRequest = {
  discriminator: 'add_decl';
  new_location: Location;
  params: CreateFunctionParams;
}

export type RemoveItemEditRequest = {
  target: number[];
};

type EditRequest =
  | MoveEditRequest
  | ResizeEditRequest
  | RenameDeclarationEditRequest
  | UpdateConstantEditRequest
  | UpdateOperatorEditRequest
  | AddConduitEditRequest
  | AddNodeEditRequest
  | AddDeclEditRequest
  | RemoveItemEditRequest;

export default EditRequest;

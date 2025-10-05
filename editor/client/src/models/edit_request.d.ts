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

export type NodeOptions = 'F64' | 'BinaryOperator' | 'UnaryOperator';

export type AddNodeEditRequest = {
  discriminator: 'add_node';
  parent: number[];
  new_type: NodeOptions;
  new_location: Location;
};

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
  | RemoveItemEditRequest;

export default EditRequest;

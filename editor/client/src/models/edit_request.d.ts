import { Location } from './fluir_module';

export type MoveEditRequest = {
  discriminator: 'move';
  target: number[];
  x: number;
  y: number;
};

export type UpdateConstantEditRequest = {
  discriminator: 'update_constant';
  target: number[];
  value: string;
};

export type AddConduitEditRequest = {
  discriminator: 'add_conduit';
  source: string; // "input-QualifiedID-index"
  target: string; // "output-QualifiedID-index"
};

export type AddNodeEditRequest = {
  discriminator: 'add_node';
  parent: number[];
  new_type: 'Constant' | 'BinaryOperator' | 'UnaryOperator';
  new_location: Location;
};

export type RemoveItemEditRequest = {
  target: number[];
};

type EditRequest =
  | MoveEditRequest
  | UpdateConstantEditRequest
  | AddConduitEditRequest
  | AddNodeEditRequest
  | RemoveItemEditRequest;

export default EditRequest;

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

type EditRequest = MoveEditRequest | UpdateConstantEditRequest;

export default EditRequest;

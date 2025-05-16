export type MoveEditRequest = {
  discriminator: 'move';
  target: number[];
  x: number;
  y: number;
};

type EditRequest = MoveEditRequest;

export default EditRequest;

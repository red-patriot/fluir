export type MoveEditRequest = {
  _t: 'move';
  target: number[];
  x: number;
  y: number;
};

type EditRequest = MoveEditRequest;

export default EditRequest;

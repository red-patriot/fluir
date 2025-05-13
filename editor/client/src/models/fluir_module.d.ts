export type Location = {
  x: number;
  y: number;
  z: number;
  width: number;
  height: number;
};

export type FlType = 'FLOATING_POINT';

export type Constant = {
  _t: 'constant';
  id: number;
  location: Location;
  flType?: FlType;
  value?: string;
};

type Operator = ' ' | '+' | '-' | '*' | '/';

export type BinaryOp = {
  _t: 'binary';
  id: number;
  location: Location;
  op: Operator;
  lhs: number;
  rhs: number;
};

export type UnaryOp = {
  _t: 'unary';
  id: number;
  location: Location;
  op: Operator;
  lhs: number;
};

export type Node = BinaryOp | UnaryOp | Constant;

export type FunctionDecl = {
  _t: 'function';
  name: string;
  id: number;
  location: Location;
  body: Node[];
};

export type Declaration = FunctionDecl;

type FluirModule = {
  declarations: Declaration[];
};

export default FluirModule;

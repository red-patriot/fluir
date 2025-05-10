export type Location = {
  x: number;
  y: number;
  z: number;
  width: number;
  height: number;
};

export type Constant = {
  id: number;
  location: Location;
  value?: number;
};

type Operator = ' ' | '+' | '-' | '*' | '/';

export type BinaryOp = {
  id: number;
  location: Location;
  op: Operator;
  lhs: number;
  rhs: number;
};

export type UnaryOp = {
  id: number;
  location: Location;
  op: Operator;
  lhs: number;
};

export type Node = BinaryOp | UnaryOp | Constant;

export type FunctionDecl = {
  name: string;
  id: number;
  location: Location;
  body: Map<number, Node>;
};

export type Declaration = FunctionDecl;

type FluirModule = {
  declarations: Declaration[];
};

export default FluirModule;

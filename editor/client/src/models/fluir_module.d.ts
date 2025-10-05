export type Location = {
  x: number;
  y: number;
  z: number;
  width: number;
  height: number;
};

export type FlType =
  | 'F64'
  | 'I8'
  | 'I16'
  | 'I32'
  | 'I64'
  | 'U8'
  | 'U16'
  | 'U32'
  | 'U64';

export type Constant = {
  discriminator: 'constant';
  id: number;
  location: Location;
  flType?: FlType;
  value?: string;
};

export type Operator = ' ' | '+' | '-' | '*' | '/';

export type BinaryOp = {
  discriminator: 'binary';
  id: number;
  location: Location;
  op: Operator;
};

export type UnaryOp = {
  discriminator: 'unary';
  id: number;
  location: Location;
  op: Operator;
};

export type Node = BinaryOp | UnaryOp | Constant;

export namespace Conduit {
  export type Output = {
    discriminator: 'conduit_output';
    target: number;
    index: number;
  };

  export type Segment = {
    discriminator: 'conduit_segment';
    x: number;
    y: number;
    children: (Segment | Output)[];
  };
}

export type Conduit = {
  discriminator: 'conduit';
  id: number;
  input: number;
  children: (Conduit.Segment | Conduit.Output)[];
};

export type FunctionDecl = {
  discriminator: 'function';
  name: string;
  id: number;
  location: Location;
  nodes: Node[];
  conduits: Conduit[];
};

export type Declaration = FunctionDecl;

type FluirModule = {
  declarations: Declaration[];
};

export default FluirModule;

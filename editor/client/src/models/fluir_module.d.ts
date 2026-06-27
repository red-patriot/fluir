export type Location = {
  x: number;
  y: number;
  z: number;
  width: number;
  height: number;
};

export type FlType =
  | "F64"
  | "I8"
  | "I16"
  | "I32"
  | "I64"
  | "U8"
  | "U16"
  | "U32"
  | "U64"
  | "BOOL";

export type Constant = {
  discriminator: "constant";
  id: number;
  location: Location;
  flType?: FlType;
  value?: string;
};

export type Operator =
  | " "
  | "+"
  | "-"
  | "*"
  | "/"
  | "++"
  | "--"
  | "=="
  | "!="
  | ">"
  | "<"
  | ">="
  | "<="
  | "!"
  | "&&"
  | "||";

export type BinaryOp = {
  discriminator: "binary";
  id: number;
  location: Location;
  op: Operator;
};

export type UnaryOp = {
  discriminator: "unary";
  id: number;
  location: Location;
  op: Operator;
};

export type Call = {
  discriminator: "call";
  target: string;
  id: number;
  location: Location;
  arguments: string[];
  returns: boolean;
};

export type Node = BinaryOp | UnaryOp | Constant | Call;

export type Comment = {
  discriminator: "comment";
  id: number;
  location: Location;
  data: string;
};

export namespace Conduit {
  export type Output = {
    discriminator: "conduit_output";
    target: number;
    index: number;
  };

  export type Segment = {
    discriminator: "conduit_segment";
    x: number;
    y: number;
    children: (Segment | Output)[];
  };
}

export type Conduit = {
  discriminator: "conduit";
  id: number;
  input: number;
  children: (Conduit.Segment | Conduit.Output)[];
};

export type FunctionParameter = {
  id: number;
  name: string;
  flType: FlType;
};

export type FunctionReturn = {
  id: number;
  flType: FlType;
};

export type FunctionDecl = {
  discriminator: "function";
  name: string;
  id: number;
  location: Location;
  nodes: Node[];
  conduits: Conduit[];
  inputs: FunctionParameter[];
  outputs: FunctionReturn[];
  annotations: Comment[];
};

export type Declaration = FunctionDecl;

type FluirModule = {
  declarations: Declaration[];
  annotations: Comment[];
};

export default FluirModule;

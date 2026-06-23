export type CompletionRequest = {
  block_id: number[];
  path: string;
};

export type TypesRequest = {
  block_id: number[];
  path: string;
};

export type OperatorsRequest = {
  operator_id: number[];
  arity: 1 | 2;
  path: string;
};

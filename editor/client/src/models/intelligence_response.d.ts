export type CompletionKind =
  | 'function'
  | 'call'
  | 'constant'
  | 'operator';

export type Completion = {
  short_name: string;
  kind: CompletionKind;
  description: string;
};

export type TypesResponse = string[];

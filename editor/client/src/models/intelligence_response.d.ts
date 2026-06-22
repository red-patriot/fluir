import { FlType } from "@/models/fluir_module";

export type CompletionKind =
  | "function"
  | "call"
  | "constant"
  | "operator"
  | "comment";

export type CompletionConstantData = {
  kind: "constant";
  value?: string;
  flType?: FlType;
};

export type CompletionOperatorData = {
  kind: "operator";
  arity: 1 | 2;
};

export type CompletionNoData = {
  kind: CompletionKind;
};

export type Completion = {
  short_name: string;
  description: string;
  data: CompletionOperatorData | CompletionConstantData | CompletionNoData;
};

export type TypesResponse = string[];

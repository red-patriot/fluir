import { useCallback } from "react";
import { SERVER_API } from "@/api";
import {
  CompletionRequest,
  OperatorsRequest,
  TypesRequest,
} from "@/models/intelligence_request";
import { Operator } from "@/models/fluir_module";

import axios, { AxiosResponse, AxiosError } from "axios";

interface UseProgramIntelligenceParams {
  handleResponse: (arg0: AxiosResponse) => void;
  onError: (arg0: AxiosError) => void;
}

export function useProgramIntelligence({
  handleResponse,
  onError,
}: UseProgramIntelligenceParams) {
  const doGetCompletions = (where: number[], program_path: string) => {
    axios
      .post(SERVER_API.completions, {
        block_id: where,
        path: program_path,
      } as CompletionRequest)
      .then(handleResponse)
      .catch(onError);
  };

  const doGetOperators = (
    where: number[],
    arity: 1 | 2,
    program_path: string,
  ): Promise<Operator[]> =>
    axios
      .post(SERVER_API.operators, {
        operator_id: where,
        arity,
        path: program_path,
      } as OperatorsRequest)
      .then((response) => response.data);

  const getCompletions = useCallback(doGetCompletions, []);
  const getOperators = useCallback(doGetOperators, []);

  return { getCompletions, getOperators };
}

export function useProgramTypes({
  handleResponse,
  onError,
}: UseProgramIntelligenceParams) {
  const doGetTypes = (where: number[], program_path: string) => {
    axios
      .post(SERVER_API.types, {
        block_id: where,
        path: program_path,
      } as TypesRequest)
      .then(handleResponse)
      .catch(onError);
  };

  const getTypes = useCallback(doGetTypes, []);

  return { getTypes };
}

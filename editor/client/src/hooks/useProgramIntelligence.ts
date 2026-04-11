import { useCallback } from 'react';
import { SERVER_API } from '@/api';
import { CompletionRequest, TypesRequest } from '@/models/intelligence_request';

import axios, { AxiosResponse, AxiosError } from 'axios';

interface UseProgramIntelligenceParams {
  handleResponse: (arg0: AxiosResponse) => void;
  onError: (arg0: AxiosError) => void;
}

export function useProgramIntelligence({ handleResponse, onError }: UseProgramIntelligenceParams) {
  const doGetCompletions = (where: number[], program_path: string) => {
    axios
      .post(SERVER_API.completions,
        {
          block_id: where,
          path: program_path,
        } as CompletionRequest)
      .then(handleResponse)
      .catch(onError);
  };

  const getCompletions = useCallback(doGetCompletions, []);

  return { getCompletions };
}

export function useProgramTypes({ handleResponse, onError }: UseProgramIntelligenceParams) {
  const doGetTypes = (where: number[], program_path: string) => {
    axios.post(SERVER_API.types,
      { block_id: where, path: program_path } as TypesRequest)
      .then(handleResponse)
      .catch(onError);
  };

  const getTypes = useCallback(doGetTypes, []);

  return { getTypes };
}

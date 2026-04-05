import { useCallback } from 'react';
import { SERVER_API } from '../api';
import { CompletionRequest } from '@/models/intelligence_request';

import axios, { AxiosResponse, AxiosError } from 'axios';

interface UseProgramIntelligenceParams {
  handleCompletions: (arg0: AxiosResponse) => void;
  onError: (arg0: AxiosError) => void;
}

export function useProgramIntelligence({ handleCompletions, onError }: UseProgramIntelligenceParams) {
  const doGetCompletions = (where: number[], program_path: string) => {
    axios
      .post(SERVER_API.completions,
        {
          block_id: where,
          path: program_path,
        } as CompletionRequest)
      .then(handleCompletions)
      .catch(onError);
  };

  const getCompletions = useCallback(doGetCompletions, []);

  return { getCompletions };
}

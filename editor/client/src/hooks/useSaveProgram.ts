import { useCallback } from 'react';
import { SERVER_API } from '../api';

import axios, { AxiosResponse, AxiosError } from 'axios';

interface UseSaveFileAsParams {
  onSave?: (arg0: AxiosResponse) => void;
  onError?: (arg0: AxiosError) => void;
}

export function useSaveFileAs({ onSave, onError }: UseSaveFileAsParams) {
  const doSaveFileAs = (programPath: string) => {
    axios
      .post(SERVER_API.saveAs, {
        path: programPath,
      })
      .then(onSave)
      .catch(onError);
  };

  const openProgram = useCallback(doSaveFileAs, []);

  return openProgram;
}

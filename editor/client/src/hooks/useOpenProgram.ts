import { useCallback } from 'react';
import { SERVER_API } from '../api';

import axios, { AxiosResponse, AxiosError } from 'axios';

interface UseOpenProgramParams {
  onOpen: (arg0: AxiosResponse) => void;
  onError: (arg0: AxiosError) => void;
}

export function useOpenProgram({ onOpen, onError }: UseOpenProgramParams) {
  const doOpenProgram = (programPath: string) => {
    axios
      .post(SERVER_API.openProgram, {
        path: programPath,
      })
      .then(onOpen)
      .catch(onError);
  };

  const openProgram = useCallback(doOpenProgram, []);

  return openProgram;
}

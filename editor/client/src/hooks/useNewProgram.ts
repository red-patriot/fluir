import { useCallback } from 'react';
import { SERVER_API } from '../api';

import axios, { AxiosResponse, AxiosError } from 'axios';

interface UseOpenProgramParams {
  onOpen: (arg0: AxiosResponse) => void;
  onError: (arg0: AxiosError) => void;
}

export function useNewProgram({ onOpen, onError }: UseOpenProgramParams) {
  const doNewProgram = () => {
    axios.post(SERVER_API.newProgram).then(onOpen).catch(onError);
  };

  const newProgram = useCallback(doNewProgram, []);

  return newProgram;
}

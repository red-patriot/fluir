import { useCallback } from 'react';
import { SERVER_API } from '../api';
import EditRequest from '../models/edit_request';

import axios, { AxiosResponse, AxiosError } from 'axios';

interface UseEditProgramParams {
  onEdit: (arg0: AxiosResponse) => void;
  onError: (arg0: AxiosError) => void;
}

export function useEditProgram({ onEdit, onError }: UseEditProgramParams) {
  const doEditProgram = (request: EditRequest) => {
    axios.post(SERVER_API.editProgram, request).then(onEdit).catch(onError);
  };

  const editProgram = useCallback(doEditProgram, []);

  return editProgram;
}

export function useUndo({ onEdit, onError }: UseEditProgramParams) {
  const doUndo = () => {
    axios.post(SERVER_API.undo).then(onEdit).catch(onError);
  };

  const undoProgramEdit = useCallback(doUndo, []);

  return undoProgramEdit;
}

export function useRedo({ onEdit, onError }: UseEditProgramParams) {
  const doRedo = () => {
    axios.post(SERVER_API.redo).then(onEdit).catch(onError);
  };

  const redoProgramEdit = useCallback(doRedo, []);

  return redoProgramEdit;
}

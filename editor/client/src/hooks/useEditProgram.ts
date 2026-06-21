import { SERVER_API } from '../api';
import EditRequest from '../models/edit_request';

import axios, { AxiosResponse, AxiosError } from 'axios';

interface UseEditProgramParams {
  onEdit: (arg0: AxiosResponse) => void;
  onError: (arg0: AxiosError) => void;
}

export function useEditProgram({ onEdit, onError }: UseEditProgramParams) {
  return (request: EditRequest) => {
    console.log("REQUEST: ", request);
    axios.post(SERVER_API.editProgram, request).then(onEdit).catch(onError);
  };
}

export function useUndo({ onEdit, onError }: UseEditProgramParams) {
  return () => {
    axios.post(SERVER_API.undo).then(onEdit).catch(onError);
  };
}

export function useRedo({ onEdit, onError }: UseEditProgramParams) {
  return () => {
    axios.post(SERVER_API.redo).then(onEdit).catch(onError);
  };
}

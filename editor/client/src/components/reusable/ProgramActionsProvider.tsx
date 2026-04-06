import { PropsWithChildren } from 'react';
import { useNewProgram } from '@/hooks/useNewProgram.ts';
import { useOpenProgram } from '@/hooks/useOpenProgram.ts';
import { useEditProgram, useRedo, useUndo } from '@/hooks/useEditProgram.ts';
import { useSaveFileAs } from '@/hooks/useSaveProgram.ts';
import { ProgramActionsContext } from './ProgramActionsContext';
import { useAppDispatch, actions } from '@/store';
import { AxiosError } from 'axios';

export default function ProgramActionsProvider({
                                                 children,
                                               }: PropsWithChildren) {
  const dispatch = useAppDispatch();
  const onError = (error: AxiosError) => {
    // TODO: Better handling here
    console.log(error);
  };

  const newProgram = useNewProgram({
    onOpen: (response) => {
      dispatch(actions.setModuleState(response.data));
      dispatch(actions.goToPage('module'));
    },
    onError,
  });

  // Local functions
  const openProgram = useOpenProgram({
    onOpen: (response) => {
      dispatch(actions.setModuleState(response.data));
      dispatch(actions.goToPage('module'));
    },
    onError,
  });

  const editProgram = useEditProgram({
    onEdit: (response) => {
      console.log(response);
      dispatch(actions.setModuleState(response.data));
    },
    onError,
  });

  const undoEdit = useUndo({
    onEdit: (response) => {
      console.log(response);
      dispatch(actions.setModuleState(response.data));
    },
    onError,
  });

  const redoEdit = useRedo({
    onEdit: (response) => {
      console.log(response);
      dispatch(actions.setModuleState(response.data));
    },
    onError,
  });

  const saveProgramAs = useSaveFileAs({
    onSave: (response) => {
      // TODO: Do something better here
      dispatch(actions.setModuleState(response.data));
      console.log(response);
    },
    onError,
  });

  return (
    <ProgramActionsContext.Provider
      value={{
        newProgram,
        openProgram,
        editProgram,
        saveProgramAs,
        undoEdit,
        redoEdit,
      }}
    >
      {children}
    </ProgramActionsContext.Provider>
  );
}

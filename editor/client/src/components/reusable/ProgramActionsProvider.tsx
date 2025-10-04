import { PropsWithChildren } from 'react';
import { useNewProgram } from '../../hooks/useNewProgram';
import { useOpenProgram } from '../../hooks/useOpenProgram';
import { useEditProgram, useRedo, useUndo } from '../../hooks/useEditProgram';
import { useSaveFileAs } from '../../hooks/useSaveProgram';
import { ProgramActionsContext } from './ProgramActionsContext';
import { useAppDispatch, actions } from '../../store';

export default function ProgramActionsProvider({
  children,
}: PropsWithChildren) {
  const dispatch = useAppDispatch();

  const newProgram = useNewProgram({
    onOpen: (response) => {
      dispatch(actions.setModuleState(response.data));
      dispatch(actions.goToPage('module'));
    },
    onError: (error) => {
      console.log(error);
    },
  });

  // Local functions
  const openProgram = useOpenProgram({
    onOpen: (response) => {
      dispatch(actions.setModuleState(response.data));
      dispatch(actions.goToPage('module'));
    },
    onError: (error) => {
      console.log(error);
    },
  });

  const editProgram = useEditProgram({
    onEdit: (response) => {
      console.log(response);
      dispatch(actions.setModuleState(response.data));
    },
    onError: (error) => {
      console.log(error);
    },
  });

  const undoEdit = useUndo({
    onEdit: (response) => {
      console.log(response);
      dispatch(actions.setModuleState(response.data));
    },
    onError: (error) => {
      console.log(error);
    },
  });

  const redoEdit = useRedo({
    onEdit: (response) => {
      console.log(response);
      dispatch(actions.setModuleState(response.data));
    },
    onError: (error) => {
      console.log(error);
    },
  });

  const saveProgramAs = useSaveFileAs({
    onSave: (response) => {
      // TODO: Do something better here
      dispatch(actions.setModuleState(response.data));
      console.log(response);
    },
    onError: (error) => {
      // TODO: Better handling here
      console.log(error);
    },
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

import { createContext, useContext } from 'react';
import EditRequest from '../../models/edit_request';

export interface ProgramActions {
  openProgram: (programPath: string) => void;
  saveProgramAs: (programPath: string) => void;
  editProgram: (request: EditRequest) => void;
}

const ProgramActionsContext = createContext<ProgramActions | undefined>(
  undefined,
);

export const useProgramActions = (): ProgramActions => {
  const ctx = useContext(ProgramActionsContext);
  if (!ctx) {
    throw new Error(
      'useProgramActions must be used within a ProgramActionsProvider',
    );
  }

  return ctx;
};

export { ProgramActionsContext };

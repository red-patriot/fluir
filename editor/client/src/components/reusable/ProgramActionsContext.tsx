import { createContext, useContext } from 'react';
import EditRequest from '../../models/edit_request';

export interface ProgramActions {
  // --- CRUD Operations ---
  newProgram: () => void;
  openProgram: (programPath: string) => void;
  saveProgramAs: (programPath: string) => void;
  editProgram: (request: EditRequest) => void;
  undoEdit: () => void;
  redoEdit: () => void;
  // --- Intelligence Operations ---
  getCompletions: (where: number[], program_path: string) => void;
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

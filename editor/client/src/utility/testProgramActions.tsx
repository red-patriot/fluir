import { ProgramActions } from '../components/common/ProgramActionsContext';
import { vi } from 'vitest';

export const mockActions: ProgramActions = {
  newProgram: vi.fn(),
  editProgram: vi.fn(),
  openProgram: vi.fn(),
  saveProgramAs: vi.fn(),
  undoEdit: vi.fn(),
  redoEdit: vi.fn(),
};

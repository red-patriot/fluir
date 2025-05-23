import { ProgramActions } from '../components/common/ProgramActionsContext';
import { vi } from 'vitest';

export const mockActions: ProgramActions = {
  editProgram: vi.fn(),
  openProgram: vi.fn(),
  saveProgramAs: vi.fn(),
};

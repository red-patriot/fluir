import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import FluirModule from '../models/fluir_module';
import { ProgramStatus } from '../models/edit_response';

export const MAX_ZOOM = 100;
export const MIN_ZOOM = 1;

export interface ProgramState {
  module?: FluirModule | null;
  path?: string;
  saved: boolean;
  canUndo: boolean;
  canRedo: boolean;
}

const initialState: ProgramState = {
  module: null,
  path: '',
  saved: false,
  canUndo: false,
  canRedo: false,
};

export const programSlice = createSlice({
  name: 'program',
  initialState,
  reducers: {
    setModuleState: (state, action: PayloadAction<ProgramStatus>) => {
      state.module = action.payload.program;
      state.path = action.payload.path;
      state.saved = action.payload.saved;
      state.canUndo = action.payload.can_undo;
      state.canRedo = action.payload.can_redo;
    },
    setOpenModule: (state, action: PayloadAction<FluirModule>) => {
      state.module = action.payload;
    },
    closeModule: (state) => {
      state.module = null;
    },
  },
});

export const { setModuleState, setOpenModule, closeModule } =
  programSlice.actions;
export default programSlice.reducer;

import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import FluirModule from '../models/fluir_module';

export interface ProgramState {
  module: FluirModule | null;
}

const initialState: ProgramState = {
  module: null,
};

export const programSlice = createSlice({
  name: 'program',
  initialState,
  reducers: {
    setOpenModule: (state, action: PayloadAction<FluirModule>) => {
      state.module = action.payload;
    },
    closeModule: (state) => {
      state.module = null;
    },
  },
});

export const { setOpenModule, closeModule } = programSlice.actions;
export default programSlice.reducer;

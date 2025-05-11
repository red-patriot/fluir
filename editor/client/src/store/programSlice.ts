import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import FluirModule from '../models/fluir_module';

export const MAX_ZOOM = 100;
export const MIN_ZOOM = 1;

export interface ProgramState {
  module: FluirModule | null;
  zoom: number;
}

const initialState: ProgramState = {
  module: null,
  zoom: 10,
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
    zoomIn: (state, action: PayloadAction<number>) => {
      state.zoom = Math.min(state.zoom * action.payload, MAX_ZOOM);
    },
    zoomOut: (state, action: PayloadAction<number>) => {
      state.zoom = Math.max(state.zoom / action.payload, MIN_ZOOM);
    },
  },
});

export const { setOpenModule, closeModule, zoomIn, zoomOut } =
  programSlice.actions;
export default programSlice.reducer;

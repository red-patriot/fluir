import { createSlice, PayloadAction } from '@reduxjs/toolkit';

export interface UiState {
  page: 'home' | 'module';
}

const initialState: UiState = {
  page: 'home',
};

export const uiSlice = createSlice({
  name: 'ui',
  initialState,
  reducers: {
    goToPage: (state, action: PayloadAction<'home' | 'module'>) => {
      state.page = action.payload;
    },
  },
});

export const { goToPage } = uiSlice.actions;
export default uiSlice.reducer;

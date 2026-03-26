import { createSlice, PayloadAction } from '@reduxjs/toolkit';

export interface UiState {
  page: 'home' | 'module';
  typeColors: Record<string, string>;
}

const initialState: UiState = {
  page: 'home',
  typeColors: {
    F64: '#FF6B6B',
    I8: '#77DD77',
    I16: '#6B9BFF',
    I32: '#4ECDC4',
    I64: '#C39BD3',
    U8: '#F7DC6F',
    U16: '#F0B27A',
    U32: '#85C1E9',
    U64: '#E59866',
  },
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

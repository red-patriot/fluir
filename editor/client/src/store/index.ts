import { combineReducers, configureStore } from '@reduxjs/toolkit';
import uiReducer, { uiSlice } from './uiSlice';
import programReducer, { programSlice } from './programSlice';
import { useDispatch, useSelector } from 'react-redux';

const rootReducer = combineReducers({
  ui: uiReducer,
  program: programReducer,
});

export function setupStore(preloadedState?: Partial<AppState>) {
  return configureStore({
    reducer: rootReducer,
    preloadedState,
  });
}

export type AppState = ReturnType<typeof rootReducer>;
export type AppStore = ReturnType<typeof setupStore>;
export type AppDispatch = AppStore['dispatch'];

export const useAppSelector = useSelector.withTypes<AppState>();
export const useAppDispatch = useDispatch.withTypes<AppDispatch>();

export const actions = {...uiSlice.actions, ...programSlice.actions};

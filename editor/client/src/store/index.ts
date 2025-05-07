import { combineReducers, configureStore } from '@reduxjs/toolkit';
import uiReducer from './uiSlice';
import { useDispatch, useSelector } from 'react-redux';

const rootReducer = combineReducers({
  ui: uiReducer,
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

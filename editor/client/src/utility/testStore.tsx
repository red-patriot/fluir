import React, { PropsWithChildren } from 'react';
import uiReducer from '../store/uiSlice';
import { configureStore } from '@reduxjs/toolkit';
import { render, RenderOptions } from '@testing-library/react';
import { AppState, setupStore } from '../store';
import { Provider } from 'react-redux';
import { ProgramActionsContext } from '../components/reusable/ProgramActionsContext';
import { mockActions } from './testProgramActions';

export const createTestStore = (preloadedState = {}) => {
  configureStore({
    reducer: {
      ui: uiReducer,
    },
    preloadedState,
  });
};

interface ExtendedRenderOptions extends Omit<RenderOptions, 'queries'> {
  preloadedState?: Partial<AppState>;
}

export function renderWithStore(
  element: React.ReactElement,
  extendedRenderOptions: ExtendedRenderOptions = {},
) {
  const { preloadedState, ...renderOptions } = extendedRenderOptions;

  const store = setupStore(preloadedState);

  const wrapWithRedux = ({ children }: PropsWithChildren) => (
    <Provider store={store}>
      <ProgramActionsContext.Provider value={mockActions}>
        {children}
      </ProgramActionsContext.Provider>
    </Provider>
  );

  return {
    store,
    ...render(element, { wrapper: wrapWithRedux, ...renderOptions }),
  };
}

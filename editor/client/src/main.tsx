import React from 'react';
import ReactDOM from 'react-dom/client';
import App from './App.tsx';
import '@xyflow/react/dist/base.css';
import './index.css';
import '@radix-ui/themes/styles.css';
import { Provider } from 'react-redux';
import { setupStore } from '@/store/index.ts';
import ProgramActionsProvider from '@/components/reusable/ProgramActionsProvider.tsx';

const store = setupStore();

ReactDOM.createRoot(document.getElementById('root')!).render(
  <React.StrictMode>
    <Provider store={store}>
      <ProgramActionsProvider>
        <App />
      </ProgramActionsProvider>
    </Provider>
  </React.StrictMode>,
);

// Use contextBridge
window.ipcRenderer.on('main-process-message', (_event, message) => {
  console.log(message);
});

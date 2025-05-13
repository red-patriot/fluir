import { IpcRenderer } from 'electron';

export interface ElectronAPI extends IpcRenderer {
  openFileDialog: () => Promise<string | null>;
  saveAsDialog: () => Promise<string | null>;
}

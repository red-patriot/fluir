import { ElectronAPI } from '../models/interface';

export const openFileDialog = () => {
  return (window.ipcRenderer as ElectronAPI).openFileDialog();
};

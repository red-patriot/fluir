import { ElectronAPI } from '../models/interface';

export const openFileDialog = () => {
  return (window.ipcRenderer as ElectronAPI).openFileDialog();
};

export const saveAsDialog = () => {
  return (window.ipcRenderer as ElectronAPI).saveAsDialog();
};

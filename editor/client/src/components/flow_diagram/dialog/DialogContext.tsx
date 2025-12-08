import { Location } from '@/models/fluir_module';
import { createContext, useContext } from 'react';
import { XYPosition } from '@xyflow/react';

export interface CreateNodeOptions {
  parentID: string;
  parentLocation: Location;
  clickedLocation: XYPosition;
  where: { x: number; y: number };
}

export interface DialogState {
  active: null | 'create_node';
  data?: CreateNodeOptions;
}

export interface DialogActions {
  closeDialog: () => void;
  openCreateNodeDialog: (opts: CreateNodeOptions) => void;
}

export const DialogContext = createContext<DialogActions | undefined>(
  undefined,
);

export const useDialogContext = (): DialogActions => {
  const ctx = useContext(DialogContext);
  if (!ctx) {
    throw new Error('useDialogContext must be used within a DialogProvider');
  }
  return ctx;
};

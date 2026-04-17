import { Location } from '@/models/fluir_module';
import { createContext, useContext } from 'react';
import { XYPosition } from '@xyflow/react';

export interface CreateNodeOptions {
  parentID: string;
  parentLocation: Location;
  clickedLocation: XYPosition;
  where: { x: number; y: number };
}

export interface TypeDialogOptions {
  parentID: string;
  where: { x: number; y: number };
  onAccept: (type: string) => void;
}

export interface DialogState {
  active: CreateNodeOptions | TypeDialogOptions | null;
}

export interface DialogActions {
  closeDialog: () => void;
  openCreateNodeDialog: (opts: CreateNodeOptions) => void;
  openTypeOptionsDialog: (opts: TypeDialogOptions) => void;
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

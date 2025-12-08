import { PropsWithChildren, useState } from 'react';

import {
  DialogState,
  CreateNodeOptions,
  DialogContext,
} from '@/components/flow_diagram/dialog/DialogContext';
import CreateNodeDialog from '@/components/flow_diagram/dialog/CreateNodeDialog';

export default function DialogProvider({ children }: PropsWithChildren) {
  const [dialogState, setDialogState] = useState<DialogState>({ active: null });

  const closeDialog = () => {
    setDialogState({ active: null, data: undefined });
  };

  const openCreateNodeDialog = (opts: CreateNodeOptions) => {
    setDialogState({ active: 'create_node', data: opts });
  };

  return (
    <DialogContext.Provider value={{ closeDialog, openCreateNodeDialog }}>
      {dialogState.active === 'create_node' && (
        <CreateNodeDialog {...(dialogState.data as CreateNodeOptions)} />
      )}
      {children}
    </DialogContext.Provider>
  );
}

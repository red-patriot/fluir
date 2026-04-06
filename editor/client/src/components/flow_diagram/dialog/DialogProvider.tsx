import { PropsWithChildren, useState } from 'react';
import { useProgramIntelligence } from '@/hooks/useProgramIntelligence';
import { useAppSelector } from '@/store';

import {
  DialogState,
  CreateNodeOptions,
  DialogContext,
} from '@/components/flow_diagram/dialog/DialogContext';
import CreateNodeDialog from '@/components/flow_diagram/dialog/CreateNodeDialog';
import { toApiID } from '@/utility/idHelpers.ts';

export default function DialogProvider({ children }: PropsWithChildren) {
  const program = useAppSelector((state) => state.program.path);
  const [dialogState, setDialogState] = useState<DialogState>({ active: null });
  const { getCompletions } = useProgramIntelligence({
    handleCompletions: (response) => {
      // TODO
      console.log(response);
    },
    onError: (error) => {
      // TODO: Better error handling
      console.log(error);
    },
  });

  const closeDialog = () => {
    setDialogState({ active: null, data: undefined });
  };

  const openCreateNodeDialog = (opts: CreateNodeOptions) => {
    // In theory, it shouldn't be possible from here that program is undefined...
    // TODO: maybe handle that case better?
    getCompletions(toApiID(opts.parentID), program || '');
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

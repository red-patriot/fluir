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
import { Completion } from '@/models/intelligence_response';

export default function DialogProvider({ children }: PropsWithChildren) {
  const program = useAppSelector((state) => state.program.path);
  const [dialogState, setDialogState] = useState<DialogState>({ active: null });
  const [completions, setCompletions] = useState<Completion[]>([]);
  const { getCompletions } = useProgramIntelligence({
    handleCompletions: (response) => {
      console.log(response);
      setCompletions(response.data);
    },
    onError: (error) => {
      // TODO: Better error handling
      console.log(error);
    },
  });

  const closeDialog = () => {
    setCompletions([]);
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
        <CreateNodeDialog {...(dialogState.data as CreateNodeOptions)}
                          options={completions} />
      )}
      {children}
    </DialogContext.Provider>
  );
}

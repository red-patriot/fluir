import { PropsWithChildren, useState } from 'react';
import {
  useProgramIntelligence,
  useProgramTypes,
} from '@/hooks/useProgramIntelligence';
import { useAppSelector } from '@/store';

import {
  DialogState,
  CreateNodeOptions,
  TypeDialogOptions,
  DialogContext,
} from '@/components/flow_diagram/dialog/DialogContext';
import CreateNodeDialog from '@/components/flow_diagram/dialog/CreateNodeDialog';
import TypeOptionsDialog from '@/components/flow_diagram/dialog/TypeOptionsDialog';
import { toApiID } from '@/utility/idHelpers.ts';
import { Completion } from '@/models/intelligence_response';

function isTypeDialogOptions(
  opts: CreateNodeOptions | TypeDialogOptions,
): opts is TypeDialogOptions {
  return 'onAccept' in opts;
}

export default function DialogProvider({ children }: PropsWithChildren) {
  const program = useAppSelector((state) => state.program.path);
  const [dialogState, setDialogState] = useState<DialogState>({ active: null });
  const [completions, setCompletions] = useState<Completion[]>([]);
  const [types, setTypes] = useState<string[]>([]);
  const { getCompletions } = useProgramIntelligence({
    handleResponse: (response) => {
      setCompletions(response.data);
    },
    onError: (error) => {
      console.log(error);
    },
  });
  const { getTypes } = useProgramTypes({
    handleResponse: (response) => {
      setTypes(response.data);
    },
    onError: (error) => {
      console.log(error);
    },
  });

  const closeDialog = () => {
    setCompletions([]);
    setTypes([]);
    setDialogState({ active: null });
  };

  const openCreateNodeDialog = (opts: CreateNodeOptions) => {
    getCompletions(toApiID(opts.parentID), program || '');
    setDialogState({ active: opts });
  };

  const openTypeOptionsDialog = (opts: TypeDialogOptions) => {
    getTypes(toApiID(opts.parentID), program || '');
    setDialogState({ active: opts });
  };

  return (
    <DialogContext.Provider
      value={{ closeDialog, openCreateNodeDialog, openTypeOptionsDialog }}
    >
      {dialogState.active && isTypeDialogOptions(dialogState.active) && (
        <TypeOptionsDialog {...dialogState.active} options={types} />
      )}
      {dialogState.active && !isTypeDialogOptions(dialogState.active) && (
        <CreateNodeDialog {...dialogState.active} options={completions} />
      )}
      {children}
    </DialogContext.Provider>
  );
}

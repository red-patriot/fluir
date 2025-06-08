import FluirModule from './fluir_module';

export type ProgramStatus = {
  saved: boolean;
  path?: string;
  program: FluirModule;
  can_undo: boolean;
  can_redo: boolean;
};

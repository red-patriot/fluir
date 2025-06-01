import FluirModule from './fluir_module';

export type ProgramStatus = {
  saved: boolean;
  path?: string;
  program: FluirModule;
};

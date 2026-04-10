import React from 'react';
import { amber } from '@radix-ui/colors';
import { Flex } from '@radix-ui/themes';
import { ValueDisplay } from './ValueDisplay';
import { editWithInputField } from '@/components/flow_diagram/common/InputField.tsx';
import { validateDeclName } from '@/components/flow_diagram/logic/validateEdit';
import { useProgramActions } from '@/components/reusable/ProgramActionsContext';
import { renameDeclaration } from '@/components/flow_diagram/logic/updateNode.ts';

interface DeclHeaderProps {
  name: string;
  variant?: 'solid' | 'ghost';
  fullID: string;
}


export default function DeclHeader({
                                     name,
                                     children,
                                     fullID,
                                   }: React.PropsWithChildren<DeclHeaderProps>) {
  const { editProgram } = useProgramActions();
  const updateName = renameDeclaration(editProgram, fullID);

  const doEdit = editWithInputField({
    validate: validateDeclName,
    onValidateSucceed: updateName,
  });

  return (
    <Flex
      direction="row"
      align="center"
      className="w-full"
      style={{
        background: amber.amber8,
      }}
    >
      <ValueDisplay fullID={fullID} value={name} renderEdit={doEdit} />
      {children}
    </Flex>
  );
}

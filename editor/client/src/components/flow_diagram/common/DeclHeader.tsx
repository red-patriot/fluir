import { amber } from '@radix-ui/colors';
import { Flex, ContextMenu } from '@radix-ui/themes';
import { ValueDisplay } from './ValueDisplay';
import { editWithInputField } from '@/components/flow_diagram/common/InputField.tsx';
import { validateDeclName } from '@/components/flow_diagram/logic/validateEdit';
import { useProgramActions } from '@/components/reusable/ProgramActionsContext';
import { renameDeclaration } from '@/components/flow_diagram/logic/updateNode.ts';
import ElementTag from '@/components/flow_diagram/common/ElementTag.tsx';

interface DeclHeaderProps {
  name: string;
  variant?: 'solid' | 'ghost';
  fullID: string;
  onContextMenu?: (event: React.MouseEvent) => void;
}


export default function DeclHeader({
                                     name,
                                     children,
                                     fullID,
                                     onContextMenu,
                                   }: React.PropsWithChildren<DeclHeaderProps>) {
  const { editProgram } = useProgramActions();
  const updateName = renameDeclaration(editProgram, fullID);

  const doEdit = editWithInputField({
    validate: validateDeclName,
    onValidateSucceed: updateName,
  });

  return (
    <ContextMenu.Root>
      <ContextMenu.Trigger>
        <Flex
          direction="row"
          align="center"
          className="w-full"
          style={{
            background: amber.amber8,
          }}
          onContextMenu={onContextMenu}
        >
          <ElementTag name="fn" />
          <ValueDisplay fullID={fullID} value={name} renderEdit={doEdit} />
          {children}
        </Flex>
      </ContextMenu.Trigger>
      <ContextMenu.Content>
        <ContextMenu.Item onClick={() => console.log("TODO PARAM")}>Add Parameter</ContextMenu.Item>
        <ContextMenu.Item onClick={() => console.log("TODO RETURN")}>Add Return</ContextMenu.Item>
      </ContextMenu.Content>
    </ContextMenu.Root>
  );
}

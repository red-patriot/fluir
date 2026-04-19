import { amber } from "@radix-ui/colors";
import { Flex, ContextMenu } from "@radix-ui/themes";
import { ValueDisplay } from "./ValueDisplay";
import { editWithInputField } from "@/components/flow_diagram/common/InputField.tsx";
import { validateDeclName } from "@/components/flow_diagram/logic/validateEdit";
import { useProgramActions } from "@/components/reusable/ProgramActionsContext";
import { renameDeclaration } from "@/components/flow_diagram/logic/updateNode.ts";
import ElementTag from "@/components/flow_diagram/common/ElementTag.tsx";
import { useDialogContext } from "@/components/flow_diagram/dialog";
import {
  AddDeclInterfaceEditRequest,
  DeclParameterParams,
  DeclReturnParams,
} from "@/models/edit_request";
import { Declaration, FlType } from "@/models/fluir_module";
import { toApiID } from "@/utility/idHelpers.ts";

interface DeclHeaderProps {
  decl: Declaration;
  fullID: string;
  variant?: "solid" | "ghost";
  onContextMenu?: (event: React.MouseEvent) => void;
}

export default function DeclHeader({
  decl,
  children,
  fullID,
  onContextMenu,
}: React.PropsWithChildren<DeclHeaderProps>) {
  const { editProgram } = useProgramActions();
  const { openTypeOptionsDialog } = useDialogContext();
  const updateName = renameDeclaration(editProgram, fullID);

  const canAddReturn = decl.outputs.length === 0;

  const doEdit = editWithInputField({
    validate: validateDeclName,
    onValidateSucceed: updateName,
  });

  const openAddParamDialog = (e: React.MouseEvent) => {
    openTypeOptionsDialog({
      parentID: fullID,
      where: { x: e.clientX, y: e.clientY },
      onAccept: (type) => {
        const request: AddDeclInterfaceEditRequest = {
          discriminator: "add_decl_interface",
          flType: type as FlType, // TODO: Handle types just being names better
          parent: toApiID(fullID),
          params: {
            discriminator: "parameter",
            name: `param${decl.inputs.length + 1}`,
          } as DeclParameterParams,
        };
        editProgram(request);
      },
    });
  };

  const openAddReturnDialog = (e: React.MouseEvent) => {
    openTypeOptionsDialog({
      parentID: fullID,
      where: { x: e.clientX, y: e.clientY },
      onAccept: (type) => {
        const request: AddDeclInterfaceEditRequest = {
          discriminator: "add_decl_interface",
          flType: type as FlType,
          parent: toApiID(fullID),
          params: {
            discriminator: "return",
          } as DeclReturnParams,
        };
        editProgram(request);
      },
    });
  };

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
          <ValueDisplay fullID={fullID} value={decl.name} renderEdit={doEdit} />
          {children}
        </Flex>
      </ContextMenu.Trigger>
      <ContextMenu.Content>
        <ContextMenu.Item onClick={openAddParamDialog}>
          Add Parameter
        </ContextMenu.Item>
        <ContextMenu.Item
          onClick={canAddReturn ? openAddReturnDialog : undefined}
          disabled={!canAddReturn}
        >
          Add Return
        </ContextMenu.Item>
      </ContextMenu.Content>
    </ContextMenu.Root>
  );
}

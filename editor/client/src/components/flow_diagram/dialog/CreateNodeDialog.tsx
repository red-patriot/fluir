import { Flex, Code, Box, Badge } from "@radix-ui/themes";
import { CreateNodeOptions } from "@/components/flow_diagram/dialog/DialogContext";
import { useProgramActions } from "@/components/reusable/ProgramActionsContext";
import {
  AddNodeEditRequest,
  AddDeclEditRequest,
  ConstantParams,
  OperatorParams,
  CallParams,
} from "@/models/edit_request";
import { toApiID } from "@/utility/idHelpers";
import { LIMITS } from "@/limits";
import { Completion, CompletionKind } from "@/models/intelligence_response";
import CoreDialog, { OptionProps } from "./CoreDialog";

interface CreateNodeDialogProps extends CreateNodeOptions {
  options: Completion[];
}

function extractWidth(kind: CompletionKind) {
  switch (kind) {
    case "operator":
      return LIMITS.operator.width.min;
    case "call":
      return LIMITS.call.width.min;
    case "constant":
      return LIMITS.constant.width.min;
    case "function":
      return LIMITS.constant.width.min;
  }
}

function extractHeight(kind: CompletionKind) {
  switch (kind) {
    case "operator":
      return LIMITS.operator.height.min;
    case "call":
      return LIMITS.call.height.min;
    case "constant":
      return LIMITS.constant.height.min;
    case "function":
      return LIMITS.constant.height.min;
  }
}

function extractParameters(
  completion: Completion,
): ConstantParams | OperatorParams | CallParams {
  switch (completion.kind) {
    case "constant":
      return {
        discriminator: "constant",
        type: completion.short_name,
        // TODO: Add some way to handle the value
      } as ConstantParams;
    case "operator":
      const opInfo = completion.short_name.split(" ");
      const op = opInfo[0];
      const arity = opInfo[1].includes("b") ? "binary" : "unary";
      return {
        discriminator: "operator",
        arity,
        op,
      } as OperatorParams;
    case "call":
      return {
        discriminator: "call",
        target: completion.short_name,
      } as CallParams;
  }
  throw new Error("Invalid completion kind");
}

export default function CreateNodeDialog({
  parentID,
  parentLocation,
  clickedLocation,
  where,
  options,
}: CreateNodeDialogProps) {
  const { editProgram } = useProgramActions();
  const onSelect = (selection: Completion) => {
    console.log(selection);
    // TODO: Refactor this component to not require this unfortunate hack
    if (selection.kind === "function") {
      const request: AddDeclEditRequest = {
        discriminator: "add_decl",
        new_location: {
          x: clickedLocation.x - parentLocation.x,
          y: clickedLocation.y - parentLocation.y,
          z: parentLocation.z + 1,
          // TODO: Get some better values for this?
          width: 40,
          height: 30,
        },
        params: { discriminator: "function" },
      };
      editProgram(request);
    } else {
      const request: AddNodeEditRequest = {
        discriminator: "add_node",
        parent: toApiID(parentID),
        new_location: {
          x: clickedLocation.x - parentLocation.x,
          y: clickedLocation.y - parentLocation.y,
          z: parentLocation.z + 1,
          width: extractWidth(selection.kind),
          height: extractHeight(selection.kind),
        },
        params: extractParameters(selection),
      };
      editProgram(request);
    }
  };

  const renderOption = ({ data, highlighted }: OptionProps<Completion>) => {
    return <CreateNodeDialogOption completion={data} selected={highlighted} />;
  };

  return (
    <CoreDialog
      where={where}
      options={options}
      optionToString={(o) => o.short_name}
      onSelect={onSelect}
      renderOption={renderOption}
    />
  );
}

interface CreateNodeDialogOptionProps extends React.HTMLProps<HTMLElement> {
  completion: Completion;
  selected?: boolean;
}

export function CreateNodeDialogOption({
  completion,
  selected = false,
}: CreateNodeDialogOptionProps) {
  return (
    <Badge color={selected ? "blue" : "gray"} className="cursor-pointer w-full">
      <Flex
        direction="row"
        align="center"
        gap="2"
        p="2"
        className="w-full justify-between"
      >
        <Code size="5" color="gray">
          {completion.short_name}
        </Code>
        {/* TODO: Update the Box to contain a visual of the element to be added?*/}
        <Box />
      </Flex>
    </Badge>
  );
}

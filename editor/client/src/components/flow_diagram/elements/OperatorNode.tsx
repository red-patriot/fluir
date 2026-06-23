import { useCallback, useEffect, useState } from "react";
import { Node, NodeProps } from "@xyflow/react";
import { cyan } from "@radix-ui/colors";
import { BinaryOp, UnaryOp, Operator } from "@/models/fluir_module";
import { Flex } from "@radix-ui/themes";
import DragHandle from "@/components/flow_diagram/common/DragHandle";
import {
  NodeInput,
  NodeOutput,
} from "@/components/flow_diagram/common/NodeInOut";
import { ValueDisplay } from "@/components/flow_diagram/common/ValueDisplay";
import { ChoicePopover } from "@/components/flow_diagram/common/ChoicePopover";
import { updateOperator } from "@/components/flow_diagram/logic";
import { useProgramActions } from "@/components/reusable/ProgramActionsContext";
import { useProgramIntelligence } from "@/hooks/useProgramIntelligence.ts";
import { useAppSelector } from "@/store";
import { toApiID } from "@/utility/idHelpers";

// Edit popover for operator nodes. Mounted by ValueDisplay only while editing,
// so the fetch fires the moment the user clicks to edit. Choice state stays
// local here, ready for per-node memoization later.
function OperatorChoices({
  id,
  current,
  onDone,
  onSelect,
  fetchOperators,
}: {
  id: string;
  current: string;
  onDone: () => void;
  onSelect: (op: string) => void;
  fetchOperators: () => Promise<Operator[]>;
}) {
  const [choices, setChoices] = useState<Operator[]>([]);

  useEffect(() => {
    fetchOperators()
      .then(setChoices)
      .catch((error) => console.error(error));
  }, [fetchOperators]);

  return (
    <ChoicePopover
      id={id}
      current={current}
      choices={choices}
      onClick={onSelect}
      onDone={onDone}
    />
  );
}

type BinaryOperatorNode = Node<{ operator: BinaryOp; fullID: string }>;

export function BinaryOperatorNode({
  data: { operator, fullID },
}: NodeProps<BinaryOperatorNode>) {
  const { editProgram } = useProgramActions();
  const { getOperators } = useProgramIntelligence({
    handleResponse: () => {},
    onError: (error) => console.error(error),
  });
  const program = useAppSelector((state) => state.program.path);

  const updateOp = updateOperator(editProgram, fullID);
  const onSelect = (op: string) => {
    updateOp(op as Operator);
  };
  const fetchOperators = useCallback(
    () => getOperators(toApiID(fullID), 2, program ?? ""),
    [getOperators, fullID, program],
  );

  return (
    <Flex
      direction="row"
      height="100%"
      align="center"
      style={{ backgroundColor: cyan.cyan11 }}
    >
      <NodeInput fullID={fullID} count={2} />
      <ValueDisplay
        fullID={fullID}
        value={operator.op}
        renderEdit={(id, current, onDone) => (
          <OperatorChoices
            id={id}
            current={current}
            onDone={onDone}
            onSelect={onSelect}
            fetchOperators={fetchOperators}
          />
        )}
      />
      <DragHandle />
      <NodeOutput fullID={fullID} count={1} />
    </Flex>
  );
}

type UnaryOperatorNode = Node<{ operator: UnaryOp; fullID: string }>;

export function UnaryOperatorNode({
  data: { operator, fullID },
}: NodeProps<UnaryOperatorNode>) {
  const { editProgram } = useProgramActions();
  const { getOperators } = useProgramIntelligence({
    handleResponse: () => {},
    onError: (error) => console.error(error),
  });
  const program = useAppSelector((state) => state.program.path);

  const updateOp = updateOperator(editProgram, fullID);
  const onSelect = (op: string) => {
    updateOp(op as Operator);
  };
  const fetchOperators = useCallback(
    () => getOperators(toApiID(fullID), 1, program ?? ""),
    [getOperators, fullID, program],
  );

  return (
    <Flex
      direction="row"
      height="100%"
      align="center"
      style={{ backgroundColor: cyan.cyan11 }}
    >
      <NodeInput fullID={fullID} count={1} />
      <ValueDisplay
        fullID={fullID}
        value={operator.op}
        renderEdit={(id, current, onDone) => (
          <OperatorChoices
            id={id}
            current={current}
            onDone={onDone}
            onSelect={onSelect}
            fetchOperators={fetchOperators}
          />
        )}
      />
      <DragHandle />
      <NodeOutput fullID={fullID} count={1} />
    </Flex>
  );
}

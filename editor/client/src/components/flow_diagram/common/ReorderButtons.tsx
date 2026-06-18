import { Flex } from "@radix-ui/themes";
import { ChevronDownIcon, ChevronUpIcon } from "@heroicons/react/24/solid";

interface ReorderButtonProps {
  moveUp?: () => void;
  moveDown?: () => void;
}

export default function ReorderButtons({
  moveUp,
  moveDown,
}: ReorderButtonProps) {
  const onMoveUp = () => {
    if (moveUp) {
      moveUp();
    }
  };
  const onMoveDown = () => {
    if (moveDown) {
      moveDown();
    }
  };

  return (
    <Flex direction="column" height={"100%"} justify={"between"}>
      <ChevronUpIcon
        className="size-3"
        opacity={moveUp ? 1 : 0.5}
        cursor={moveUp ? "pointer" : "default"}
        onClick={onMoveUp}
      />
      <ChevronDownIcon
        className="size-3"
        opacity={moveDown ? 1 : 0.5}
        cursor={moveDown ? "pointer" : "default"}
        onClick={onMoveDown}
      />
    </Flex>
  );
}

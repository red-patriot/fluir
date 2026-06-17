import { Flex } from "@radix-ui/themes";
import { CaretDownIcon, CaretUpIcon } from "@radix-ui/react-icons";
import { gray } from "@radix-ui/colors";

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
    <Flex direction="column">
      <CaretUpIcon color={moveUp ? undefined : gray.gray8} onClick={onMoveUp} />
      <CaretDownIcon
        color={moveDown ? undefined : gray.gray8}
        onClick={onMoveDown}
      />
    </Flex>
  );
}

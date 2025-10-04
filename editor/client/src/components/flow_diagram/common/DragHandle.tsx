import { DragHandleDots2Icon } from '@radix-ui/react-icons';
import { Box } from '@radix-ui/themes';

export default function DragHandle() {
  return (
    <Box className='self-center cursor-move dragHandle__custom'>
      <DragHandleDots2Icon />
    </Box>
  );
}

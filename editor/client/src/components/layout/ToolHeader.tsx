import { useState } from 'react';

import IconButton from '@/components/reusable/IconButton';
import {
  FileIcon,
  Pencil2Icon,
  ResetIcon,
  ExitIcon,
} from '@radix-ui/react-icons';
import { Box, Flex, Code } from '@radix-ui/themes';
import { useAppSelector } from '../../store';

interface ToolHeaderProps {
  onSave?: () => void;
  onSaveAs?: () => void;
  onCloseModule?: () => void;
  onUndo?: () => void;
  onRedo?: () => void;
}

export default function ToolHeader({
  onSave,
  onSaveAs,
  onCloseModule,
  onUndo,
  onRedo,
}: ToolHeaderProps) {
  const modulePath = useAppSelector((state) => state.program.path);
  const saved = useAppSelector((state) => state.program.saved);
  const canUndo = useAppSelector((state) => state.program.canUndo);
  const canRedo = useAppSelector((state) => state.program.canRedo);
  const moduleName =
    modulePath?.split('/')[modulePath?.split('/').length - 1] || '<unnamed>';

  const [hovered, setIsHovered] = useState(false);
  // <div className='flex items-center justify-between p-1 bg-slate-600 text-white'>

  return (
    <Flex
      align='center'
      justify='between'
      gap='1'
      className='p-1'
    >
      <IconButton
        aria-label='module-save'
        variant='solid'
        icon={<FileIcon />}
        onClick={onSave}
        disabled={!modulePath}
      />
      <IconButton
        aria-label='module-save-as'
        variant='solid'
        icon={<Pencil2Icon />}
        onClick={onSaveAs}
      />
      <IconButton
        aria-label='module-undo'
        variant='solid'
        icon={<ResetIcon />}
        onClick={onUndo}
        disabled={!canUndo}
      />
      <IconButton
        aria-label='module-redo'
        variant='solid'
        icon={<ResetIcon className='-scale-x-100' />}
        onClick={onRedo}
        disabled={!canRedo}
      />
      <Box flexGrow='1' />
      <Code
        size='7'
        truncate
        variant='outline'
        onMouseEnter={() => setIsHovered(true)}
        onMouseLeave={() => setIsHovered(false)}
        className='cursor-default w-1/2 text-center'
      >
        {hovered ? modulePath : moduleName}
        {saved ? '' : '*'}
      </Code>
      <Box flexGrow='1' />
      <IconButton
        aria-label='module-close'
        variant='solid'
        icon={<ExitIcon />}
        color='red'
        onClick={onCloseModule}
      />
    </Flex>
  );
}

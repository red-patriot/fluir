import { useState } from 'react';

import IconButton from '../common/IconButton';
import {
  faSave,
  faFilePen,
  faArrowUpRightFromSquare,
} from '@fortawesome/free-solid-svg-icons';
import { useAppSelector } from '../../store';

interface ToolHeaderProps {
  onSave?: () => void;
  onSaveAs?: () => void;
  onCloseModule?: () => void;
}

export default function ToolHeader({
  onSave,
  onSaveAs,
  onCloseModule,
}: ToolHeaderProps) {
  const modulePath = useAppSelector((state) => state.program.path);
  const saved = useAppSelector((state) => state.program.saved);
  const moduleName =
    modulePath?.split('/')[modulePath?.split('/').length - 1] || '';

  const [hovered, setIsHovered] = useState(false);

  return (
    <div className='flex items-center justify-between p-1 bg-slate-600 text-white'>
      <IconButton
        iconProps={{ icon: faSave }}
        onClick={onSave}
      />
      <IconButton
        iconProps={{ icon: faFilePen }}
        onClick={onSaveAs}
      />
      <span className='grow' />
      <div
        className='w-1/2 h-full flex items-center justify-around
          rounded-lg cursor-default
          bg-[#1a1a1a] text-white font-code
          hover:bg-[#2a2a2a] transition-colors'
        onMouseEnter={() => setIsHovered(true)}
        onMouseLeave={() => setIsHovered(false)}
      >
        <h2>
          {hovered ? modulePath : moduleName}
          {saved ? '' : '*'}
        </h2>
      </div>
      <span className='grow' />
      <IconButton
        iconProps={{ icon: faArrowUpRightFromSquare, color: 'red' }}
        onClick={onCloseModule}
      />
    </div>
  );
}

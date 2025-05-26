import IconButton from '../common/IconButton';
import {
  faSave,
  faFilePen,
  faArrowUpRightFromSquare,
} from '@fortawesome/free-solid-svg-icons';

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
  return (
    <div className='flex items-center justify-between p-2 bg-slate-600 text-white'>
      <IconButton
        iconProps={{ icon: faSave }}
        onClick={onSave}
      />
      <IconButton
        iconProps={{ icon: faFilePen }}
        onClick={onSaveAs}
      />
      <span className='grow' />
      <IconButton
        iconProps={{ icon: faArrowUpRightFromSquare, color: 'red' }}
        onClick={onCloseModule}
      />
    </div>
  );
}

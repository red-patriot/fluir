import IconButton from '../common/IconButton';
import {
  faDoorClosed,
  faFilePen,
  faFloppyDisk,
} from '@fortawesome/free-solid-svg-icons';

interface ModuleHeaderProps {
  onSave?: () => void;
  onSaveAs?: () => void;
  onCloseModule?: () => void;
}

export default function ModuleHeader({
  onSave,
  onSaveAs,
  onCloseModule,
}: ModuleHeaderProps) {
  return (
    <div
      className='flex flex-row justify-start m-2
                        bg-gray-500 rounded-lg'
    >
      <IconButton
        label='save-button'
        iconProps={{ icon: faFloppyDisk }}
        onClick={onSave}
      />
      <IconButton
        label='save-as-button'
        iconProps={{ icon: faFilePen }}
        onClick={onSaveAs}
      />

      <span className='grow' />
      <IconButton
        label='close-module-button'
        iconProps={{ icon: faDoorClosed, color: 'red' }}
        onClick={onCloseModule}
      />
    </div>
  );
}

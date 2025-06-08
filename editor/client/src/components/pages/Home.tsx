import viteLogo from '/electron-vite.animate.svg';
import { faFileCirclePlus, faFilePen } from '@fortawesome/free-solid-svg-icons';
import IconButton from '../common/IconButton';

interface HomeProps {
  onNewFile?: () => void;
  onOpenFile?: () => void;
}

export default function Home({ onNewFile, onOpenFile }: HomeProps) {
  return (
    <div className='flex justify-center h-lvh align-bottom items-center'>
      <img
        src={viteLogo}
        className='logo size-1/3'
      />
      <div className='flex flex-col align-middle'>
        <IconButton
          aria-label='new-file-button'
          onClick={onNewFile}
          iconProps={{ icon: faFileCirclePlus }}
          text='New File'
        />
        <IconButton
          aria-label='open-file-button'
          onClick={onOpenFile}
          iconProps={{ icon: faFilePen }}
          text='Open File'
        />
      </div>
    </div>
  );
}

import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';
import viteLogo from '/electron-vite.animate.svg';
import { faFileCirclePlus, faFilePen } from '@fortawesome/free-solid-svg-icons';

interface HomeProps {
  onOpenFile: () => void;
}

export default function Home({ onOpenFile }: HomeProps) {
  return (
    <div className='flex flex-row'>
      <img
        src={viteLogo}
        className='logo'
      />
      <div className='flex flex-col grid content-evenly'>
        <button>
          <FontAwesomeIcon icon={faFileCirclePlus} />
          New File
        </button>
        <button
          aria-label='open-file-button'
          onClick={onOpenFile}
        >
          <FontAwesomeIcon icon={faFilePen} />
          Open File
        </button>
      </div>
    </div>
  );
}

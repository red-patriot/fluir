import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';
import viteLogo from '/electron-vite.animate.svg';
import { faFileCirclePlus, faFilePen } from '@fortawesome/free-solid-svg-icons';

export default function Home() {
  return (
    <div className='flex flex-row'>
      <img src={viteLogo} className='logo' />
      <div className='flex flex-col grid content-evenly'>
        <button>
          <FontAwesomeIcon icon={faFileCirclePlus} />
          New File
        </button>
        <button>
          <FontAwesomeIcon icon={faFilePen} />
          Open File
        </button>
      </div>
    </div>
  );
}

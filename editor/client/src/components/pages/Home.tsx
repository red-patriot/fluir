import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';
import viteLogo from '/electron-vite.animate.svg';
import { faFileCirclePlus, faFilePen } from '@fortawesome/free-solid-svg-icons';
import { ElectronAPI } from '../../models/interface';

export default function Home() {
  const handleOpenFile = async () => {
    const filePath = await (window.ipcRenderer as ElectronAPI).openFileDialog();
    if (filePath) {
      console.log(`Selected path ${filePath}`);
    } else {
      alert('No file selected');
    }
  };

  return (
    <div className='flex flex-row'>
      <img src={viteLogo} className='logo' />
      <div className='flex flex-col grid content-evenly'>
        <button>
          <FontAwesomeIcon icon={faFileCirclePlus} />
          New File
        </button>
        <button onClick={handleOpenFile}>
          <FontAwesomeIcon icon={faFilePen} />
          Open File
        </button>
      </div>
    </div>
  );
}

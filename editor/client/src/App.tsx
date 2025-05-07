import './App.css';
import { useAppSelector } from './store';
import Home from './components/pages/Home';
import { openFileDialog } from './hooks/electronAPI';

function App() {
  const page = useAppSelector((state) => state.ui.page);

  // TODO: Update this here
  const onOpenFile = async () => {
    const filePath = await openFileDialog();
    if (filePath) {
      console.log(`Opened: ${filePath}`);
    } else {
      // TODO: Do something better
      console.log('File selection was cancelled');
    }
  };

  return (
    <>
      {page == 'home' && <Home onOpenFile={onOpenFile} />}
      {page == 'program' && <p>TODO</p>}
    </>
  );
}

export default App;

import { useAppSelector } from './store';
import Home from './components/pages/Home';
import { openFileDialog } from './hooks/electronAPI';
import ModulePage from './components/pages/Module';
import { useProgramActions } from './components/common/ProgramActionsContext';
import { Theme } from '@radix-ui/themes';

function App() {
  // global state
  const { newProgram, openProgram } = useProgramActions();
  const page = useAppSelector((state) => state.ui.page);

  const onOpenFile = async () => {
    const filePath = await openFileDialog();
    if (filePath) {
      openProgram(filePath);
    } else {
      // TODO: Do something better
      console.log('File selection was cancelled');
    }
  };

  return (
    <Theme accentColor='blue' grayColor='gray' radius='none' appearance='dark'>
      {page == 'home' && <Home onNewFile={newProgram} onOpenFile={onOpenFile} />}
      {page == 'module' && <ModulePage />}
    </Theme>
  );
}

export default App;

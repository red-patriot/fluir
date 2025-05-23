import { useAppSelector } from './store';
import Home from './components/pages/Home';
import { openFileDialog } from './hooks/electronAPI';
import ModulePage from './components/pages/Module';
import { useProgramActions } from './components/common/ProgramActionsContext';

function App() {
  // global state
  const { openProgram, editProgram } = useProgramActions();
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
    <>
      {page == 'home' && <Home onOpenFile={onOpenFile} />}
      {page == 'module' && <ModulePage onEdit={editProgram} />}
    </>
  );
}

export default App;

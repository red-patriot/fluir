import { useAppSelector, useAppDispatch, actions } from './store';
import Home from './components/pages/Home';
import { openFileDialog } from './hooks/electronAPI';
import { useOpenProgram } from './hooks/useOpenProgram';
import { useEditProgram } from './hooks/useEditProgram';
import ModulePage from './components/pages/Module';

function App() {
  // global state
  const dispatch = useAppDispatch();
  const page = useAppSelector((state) => state.ui.page);

  // Local functions
  const openProgram = useOpenProgram({
    onOpen: (response) => {
      dispatch(actions.setOpenModule(response.data));
      dispatch(actions.goToPage('module'));
    },
    onError: (error) => {
      console.log(error);
    },
  });

  const editProgram = useEditProgram({
    onEdit: (response) => {
      console.log(response);
      dispatch(actions.setOpenModule(response.data));
    },
    onError: (error) => {
      console.log(error);
    },
  });

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

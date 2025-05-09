import './App.css';
import { useAppSelector, useAppDispatch, actions } from './store';
import Home from './components/pages/Home';
import { openFileDialog } from './hooks/electronAPI';
import { useOpenProgram } from './hooks/useOpenProgram';

function App() {
  // global state
  const dispatch = useAppDispatch();
  const page = useAppSelector((state) => state.ui.page);

  // Local functions
  const openProgram = useOpenProgram({
    onOpen: (response) => {
      dispatch(actions.setOpenModule(response.data));
      dispatch(actions.goToPage('program'));
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
      {page == 'program' && <p>TODO</p>}
    </>
  );
}

export default App;

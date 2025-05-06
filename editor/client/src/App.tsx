import './App.css';
import { useAppSelector } from './store';
import Home from './components/pages/Home';

function App() {
  const page = useAppSelector((state) => state.ui.page);

  return (
    <>
      {page == 'home' && <Home />}
      {page == 'program' && <p>TODO</p>}
    </>
  );
}

export default App;

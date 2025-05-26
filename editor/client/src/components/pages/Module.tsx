import ViewWindow from '../layout/ViewWindow';
import ToolHeader from '../layout/ToolHeader';
import { actions, useAppDispatch } from '../../store';
import { useProgramActions } from '../common/ProgramActionsContext';
import { saveAsDialog } from '../../hooks/electronAPI';

export default function ModulePage() {
  const dispatch = useAppDispatch();
  const { saveProgramAs } = useProgramActions();

  // Local functions
  const onSaveAs = async () => {
    const filePath = await saveAsDialog();
    if (filePath) {
      return saveProgramAs(filePath);
    } else {
      // TODO: Handle the error better
      console.log('File selection was cancelled');
    }
  };
  const closeModule = () => {
    dispatch(actions.closeModule());
    dispatch(actions.goToPage('home'));
  };

  const onSave = async () => {
    // Empty string saves the file in-place
    return saveProgramAs('');
  };

  return (
    <div className='flex flex-col h-lvh w-lvw'>
      <ToolHeader
        onSave={onSave}
        onSaveAs={onSaveAs}
        onCloseModule={closeModule}
      />
      <ViewWindow />
    </div>
  );
}

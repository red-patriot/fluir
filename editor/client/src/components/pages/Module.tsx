import ModuleViewWindow from '../layout/ModuleViewWindow';
import ModuleHeader from '../layout/ModuleHeader';
import EditRequest from '../../models/edit_request';
import { useSaveFileAs } from '../../hooks/useSaveProgram';
import { saveAsDialog } from '../../hooks/electronAPI';
import { useAppDispatch, actions } from '../../store';

interface ModulePageProps {
  onEdit: (arg0: EditRequest) => void;
}

export default function ModulePage({ onEdit }: ModulePageProps) {
  const dispatch = useAppDispatch();

  // Local functions
  const saveProgramAs = useSaveFileAs({
    onSave: (response) => {
      // TODO: Do something better here
      console.log(response);
    },
    onError: (error) => {
      // TODO: Better handling here
      console.log(error);
    },
  });

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
    <div className='flex flex-col h-lvh border-2'>
      <ModuleHeader
        onSaveAs={onSaveAs}
        onSave={onSave}
        onCloseModule={closeModule}
      />
      <ModuleViewWindow onEdit={onEdit} />

    </div>
  );
}

import ModuleViewWindow from '../layout/ModuleViewWindow';
import ModuleHeader from '../layout/ModuleHeader';
import EditRequest from '../../models/edit_request';

interface ModulePageProps {
  onEdit: (arg0: EditRequest) => void;
}

export default function ModulePage({ onEdit }: ModulePageProps) {
  return (
    <div className='flex flex-col h-full'>
      <ModuleHeader />
      <ModuleViewWindow onEdit={onEdit} />
    </div>
  );
}

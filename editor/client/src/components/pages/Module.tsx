import ModuleViewWindow from '../layout/ModuleViewWindow';
import EditRequest from '../../models/edit_request';

interface ModulePageProps {
  onEdit: (arg0: EditRequest) => void;
}

export default function ModulePage({ onEdit }: ModulePageProps) {
  return (
    <div className='flex flex-col h-full'>
      <ModuleViewWindow onEdit={onEdit} />
    </div>
  );
}

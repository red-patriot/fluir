import { ContextMenu } from 'radix-ui';
import { XYZPosition } from '@xyflow/react';
import EditRequest, {
  AddNodeEditRequest,
  NodeOptions,
} from '../../models/edit_request';
import { toApiID } from '../../utility/idHelpers';
import { Location } from '../../models/fluir_module';

interface AddNodeItemProps {
  name: string;
  onSelect?: () => void;
}

function AddNodeItem({ name, onSelect }: AddNodeItemProps) {
  return (
    <ContextMenu.Item
      className='hover:bg-blue-100 data-[highlighted]:bg-blue-100  rounded-lg'
      onSelect={onSelect}
    >
      {name}
    </ContextMenu.Item>
  );
}

interface AddNodeMenuProps {
  parentID: string;
  editProgram: (request: EditRequest) => void;
  addLocation?: XYZPosition;
}

export default function AddNodeMenu({
  parentID,
  editProgram,
  addLocation = { x: 0, y: 0, z: 0 },
}: AddNodeMenuProps) {
  const addNodeCallback = (newType: NodeOptions, newLocation: Location) => {
    return () => {
      const request: AddNodeEditRequest = {
        discriminator: 'add_node',
        parent: toApiID(parentID),
        new_type: newType,
        new_location: newLocation,
      };

      editProgram(request);
    };
  };

  return (
    <ContextMenu.Content className='w-xs font-[18] bg-white text-black rounded-lg'>
      <span>Add Node</span>
      <ContextMenu.Separator className='h-[1px] bg-black m-[5px]' />
      <AddNodeItem
        name='Float Constant'
        onSelect={addNodeCallback('Constant', {
          x: addLocation.x,
          y: addLocation.y,
          z: addLocation.z,
          width: 12,
          height: 5,
        })}
      />
      <AddNodeItem
        name='Binary Operator'
        onSelect={addNodeCallback('BinaryOperator', {
          x: addLocation.x,
          y: addLocation.y,
          z: addLocation.z,
          width: 5,
          height: 5,
        })}
      />
      <AddNodeItem
        name='Unary Operator'
        onSelect={addNodeCallback('UnaryOperator', {
          x: addLocation.x,
          y: addLocation.y,
          z: addLocation.z,
          width: 5,
          height: 5,
        })}
      />
    </ContextMenu.Content>
  );
}

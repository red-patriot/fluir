import { MoveEditRequest } from '@/models/edit_request';
import { toApiID } from '@/utility/idHelpers';

export function move(
  commit: (request: MoveEditRequest) => void,
  fullID: string,
) {
  return (x: number, y: number) => {
    const request: MoveEditRequest = {
      discriminator: 'move',
      target: toApiID(fullID),
      x,
      y,
    };
    commit(request);
  };
}

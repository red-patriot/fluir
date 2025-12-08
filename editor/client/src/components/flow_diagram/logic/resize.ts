import { ResizeEditRequest } from '@/models/edit_request';
import { toApiID } from '@/utility/idHelpers';

export function resizeMove(
  commit: (request: ResizeEditRequest) => void,
  fullID: string,
) {
  return (width: number, height: number, x?: number, y?: number) => {
    const request: ResizeEditRequest = {
      discriminator: 'resize',
      target: toApiID(fullID),
      width,
      height,
      x,
      y,
    };
    commit(request);
  };
}

export function resize(
  commit: (request: ResizeEditRequest) => void,
  fullID: string,
) {
  return (width: number, height: number) => {
    const request: ResizeEditRequest = {
      discriminator: 'resize',
      target: toApiID(fullID),
      width,
      height,
    };
    commit(request);
  };
}

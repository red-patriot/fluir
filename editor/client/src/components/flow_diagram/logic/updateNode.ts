import { UpdateConstantEditRequest } from '@/models/edit_request';
import { toApiID } from '@/utility/idHelpers';

export function updateConstant(
  commit: (request: UpdateConstantEditRequest) => void,
  fullID: string,
) {
  return (value: string) => {
    const request: UpdateConstantEditRequest = {
      discriminator: 'update_constant',
      target: toApiID(fullID),
      value,
    };
    commit(request);
  };
}

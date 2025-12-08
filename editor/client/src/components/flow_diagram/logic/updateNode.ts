import {
  UpdateConstantEditRequest,
  UpdateOperatorEditRequest,
} from '@/models/edit_request';
import { Operator } from '@/models/fluir_module';
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

export function updateOperator(
  commit: (request: UpdateOperatorEditRequest) => void,
  fullID: string,
) {
  return (value: Operator) => {
    const request: UpdateOperatorEditRequest = {
      discriminator: 'update_operator',
      target: toApiID(fullID),
      value,
    };
    commit(request);
  };
}

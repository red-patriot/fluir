import { describe, it, expect, vi } from 'vitest';
import { updateConstant } from '@/components/flow_diagram/logic/updateNode';
import { UpdateConstantEditRequest } from '@/models/edit_request';

describe('updateConstant', () => {
  it('Contains the correct id', () => {
    const commitMock = vi.fn();
    const id = '0:1:2';

    const uut = updateConstant(commitMock, id);

    uut('');

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: 'update_constant',
      target: [0, 1, 2],
      value: '',
    } as UpdateConstantEditRequest);
  });

  it('Sends the correct values', () => {
    const commitMock = vi.fn();
    const id = '0:1:2';

    const uut = updateConstant(commitMock, id);

    uut('145');

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: 'update_constant',
      target: [0, 1, 2],
      value: '145',
    } as UpdateConstantEditRequest);
  });
});

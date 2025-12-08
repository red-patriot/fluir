import { describe, it, expect, vi } from 'vitest';
import { move } from '@/components/flow_diagram/logic/move';
import { MoveEditRequest } from '@/models/edit_request';

describe('move', () => {
  it('Contains the correct id', () => {
    const commitMock = vi.fn();
    const id = '0:1:2';

    const uut = move(commitMock, id);

    uut(0, 0);

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: 'move',
      target: [0, 1, 2],
      x: 0,
      y: 0,
    } as MoveEditRequest);
  });

  it('Sends the correct values', () => {
    const commitMock = vi.fn();
    const id = '0:1:2';

    const uut = move(commitMock, id);

    uut(1, 2);

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: 'move',
      target: [0, 1, 2],
      x: 1,
      y: 2,
    } as MoveEditRequest);
  });
});

import { describe, it, expect, vi } from 'vitest';
import { resizeMove, resize } from '@/components/flow_diagram/logic/resize';
import { ResizeEditRequest } from '@/models/edit_request';

describe('resizeMove', () => {
  it('Contains the correct id', () => {
    const commitMock = vi.fn();
    const id = '0:1:2';

    const uut = resizeMove(commitMock, id);

    uut(0, 0, 0, 0);

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: 'resize',
      target: [0, 1, 2],
      width: 0,
      height: 0,
      x: 0,
      y: 0,
    } as ResizeEditRequest);
  });

  it('Sends the correct values', () => {
    const commitMock = vi.fn();
    const id = '0:1:2';

    const uut = resizeMove(commitMock, id);

    uut(1, 2, 3, 4);

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: 'resize',
      target: [0, 1, 2],
      width: 1,
      height: 2,
      x: 3,
      y: 4,
    } as ResizeEditRequest);
  });

  it('Optionallydoesnt send XY', () => {
    const commitMock = vi.fn();
    const id = '0:1:2';

    const uut = resizeMove(commitMock, id);

    uut(1, 2);

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: 'resize',
      target: [0, 1, 2],
      width: 1,
      height: 2,
    } as ResizeEditRequest);
  });
});

describe('resize', () => {
  it('Contains the correct id', () => {
    const commitMock = vi.fn();
    const id = '0:1:2';

    const uut = resize(commitMock, id);

    uut(0, 0);

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: 'resize',
      target: [0, 1, 2],
      width: 0,
      height: 0,
    } as ResizeEditRequest);
  });

  it('Sends the correct values', () => {
    const commitMock = vi.fn();
    const id = '0:1:2';

    const uut = resize(commitMock, id);

    uut(1, 2);

    expect(commitMock).toHaveBeenCalledOnce();
    expect(commitMock).toHaveBeenLastCalledWith({
      discriminator: 'resize',
      target: [0, 1, 2],
      width: 1,
      height: 2,
    } as ResizeEditRequest);
  });
});

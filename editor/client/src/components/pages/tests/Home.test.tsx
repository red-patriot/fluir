import { cleanup, fireEvent, screen } from '@testing-library/react';
import { renderWithStore } from '../../../utility/testStore';
import { describe, it, vi, expect, afterEach } from 'vitest';
import Home from '../Home';

describe('Home Page', () => {
  afterEach(cleanup);
  afterEach(vi.clearAllMocks);

  const onOpenFile = vi.fn();

  it('Calls the onOpenFile hook if a file is opened', async () => {
    renderWithStore(<Home onOpenFile={onOpenFile} />);

    fireEvent.click(await screen.findByLabelText('open-file-button'));

    expect(onOpenFile).toHaveBeenCalledOnce();
  });
});

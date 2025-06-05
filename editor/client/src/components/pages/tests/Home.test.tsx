import { cleanup, screen, waitFor } from '@testing-library/react';
import { renderWithStore } from '../../../utility/testStore';
import { describe, it, vi, expect, afterEach, beforeEach } from 'vitest';
import Home from '../Home';
import userEvent from '@testing-library/user-event';
import '@testing-library/jest-dom';

describe('Home Page', () => {
  afterEach(cleanup);
  beforeEach(vi.clearAllMocks);

  it('Calls the onNewFile hook if new button is pressed', async () => {
    const onNewFile = vi.fn();
    renderWithStore(<Home onNewFile={onNewFile} />);

    await userEvent.click(screen.getByLabelText('new-file-button'));
    expect(onNewFile).toHaveBeenCalledOnce();
  });

  it('Calls the onOpenFile hook if a file is opened', async () => {
    const onOpenFile = vi.fn();
    renderWithStore(<Home onOpenFile={onOpenFile} />);

    await userEvent.click(screen.getByLabelText('open-file-button'));

    expect(onOpenFile).toHaveBeenCalledOnce();
  });
});

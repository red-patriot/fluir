import { describe, it, expect, afterEach, vi } from 'vitest';
import { renderWithStore } from '../../../utility/testStore';
import { cleanup, fireEvent, screen } from '@testing-library/react';
import '@testing-library/jest-dom';
import ModuleHeader from '../ModuleHeader';

describe('ModuleHeader', () => {
  afterEach(cleanup);

  it('Calls the on-save function', async () => {
    const mockOnSave = vi.fn();

    renderWithStore(<ModuleHeader onSave={mockOnSave} />);

    fireEvent.click(await screen.getByLabelText('save-button'));

    expect(mockOnSave).toHaveBeenCalledOnce();
  });

  it('Calls the on-save-as function', async () => {
    const mockOnSave = vi.fn();

    renderWithStore(<ModuleHeader onSaveAs={mockOnSave} />);

    fireEvent.click(await screen.getByLabelText('save-as-button'));

    expect(mockOnSave).toHaveBeenCalledOnce();
  });

  it('Calls the close-module function', async () => {
    const mockOnCloseModule = vi.fn();

    renderWithStore(<ModuleHeader onCloseModule={mockOnCloseModule} />);

    fireEvent.click(await screen.getByLabelText('close-module-button'));

    expect(mockOnCloseModule).toHaveBeenCalledOnce();
  });
});

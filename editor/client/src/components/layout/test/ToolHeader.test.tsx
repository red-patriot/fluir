import { vi, it, describe, expect } from 'vitest';
import { fireEvent, screen } from '@testing-library/react';
import ToolHeader from '@/components/layout/ToolHeader';
import { renderWithStore } from '@/utility/testStore';
import FluirModule from '@/models/fluir_module';

describe('ToolHeader', () => {
  const simpleModule: FluirModule = {
    declarations: [],
  };

  it('Displays the module name', () => {
    renderWithStore(<ToolHeader />, {
      preloadedState: {
        program: {
          path: '/home/dummy/Documents/test.fl',
          module: simpleModule,
          canRedo: false,
          canUndo: false,
          saved: true,
        },
      },
    });

    expect(screen.getAllByText('test.fl')).toHaveLength(1);
  });

  it('Displays an asterisk when not saved', () => {
    renderWithStore(<ToolHeader />, {
      preloadedState: {
        program: {
          path: '/home/dummy/Documents/test.fl',
          module: simpleModule,
          canRedo: false,
          canUndo: false,
          saved: false,
        },
      },
    });

    expect(screen.getAllByText('test.fl*')).toHaveLength(1);
  });

  it('Displays a placeholder name when no path is given', () => {
    renderWithStore(<ToolHeader />, {
      preloadedState: {
        program: {
          path: '',
          module: simpleModule,
          canRedo: false,
          canUndo: false,
          saved: false,
        },
      },
    });

    expect(screen.getAllByText('<unnamed>*')).toHaveLength(1);
  });

  it('Disables Undo button when it cannot be performed', () => {
    renderWithStore(<ToolHeader />, {
      preloadedState: {
        program: {
          path: '/home/dummy/Documents/test.fl',
          module: simpleModule,
          canRedo: false,
          canUndo: false,
          saved: true,
        },
      },
    });

    expect(screen.getByLabelText('module-undo')).toBeDisabled();
  });

  it('Enables Undo button when it can be performed', () => {
    renderWithStore(<ToolHeader />, {
      preloadedState: {
        program: {
          path: '/home/dummy/Documents/test.fl',
          module: simpleModule,
          canRedo: false,
          canUndo: true,
          saved: true,
        },
      },
    });

    expect(screen.getByLabelText('module-undo')).toBeEnabled();
  });

  it('Disables Redo button when it cannot be performed', () => {
    renderWithStore(<ToolHeader />, {
      preloadedState: {
        program: {
          path: '/home/dummy/Documents/test.fl',
          module: simpleModule,
          canRedo: false,
          canUndo: false,
          saved: true,
        },
      },
    });

    expect(screen.getByLabelText('module-redo')).toBeDisabled();
  });

  it('Enables Redo button when it can be performed', () => {
    renderWithStore(<ToolHeader />, {
      preloadedState: {
        program: {
          path: '/home/dummy/Documents/test.fl',
          module: simpleModule,
          canRedo: true,
          canUndo: false,
          saved: true,
        },
      },
    });

    expect(screen.getByLabelText('module-redo')).toBeEnabled();
  });

  it('Calls onSave when save is clicked', async () => {
    const mockOnSave = vi.fn();

    renderWithStore(<ToolHeader onSave={mockOnSave} />, {
      preloadedState: {
        program: {
          path: '/home/dummy/Documents/test.fl',
          module: simpleModule,
          canRedo: true,
          canUndo: true,
          saved: false,
        },
      },
    });

    fireEvent.click(await screen.getByLabelText('module-save'));

    expect(mockOnSave).toHaveBeenCalledOnce();
  });

  it('Calls onSaveAs when save is clicked', async () => {
    const mockOnSaveAs = vi.fn();

    renderWithStore(<ToolHeader onSaveAs={mockOnSaveAs} />, {
      preloadedState: {
        program: {
          path: '/home/dummy/Documents/test.fl',
          module: simpleModule,
          canRedo: true,
          canUndo: true,
          saved: false,
        },
      },
    });

    fireEvent.click(await screen.getByLabelText('module-save-as'));

    expect(mockOnSaveAs).toHaveBeenCalledOnce();
  });

  it('Calls onUndo when undo is clicked', async () => {
    const mockUndo = vi.fn();

    renderWithStore(<ToolHeader onUndo={mockUndo} />, {
      preloadedState: {
        program: {
          path: '/home/dummy/Documents/test.fl',
          module: simpleModule,
          canRedo: true,
          canUndo: true,
          saved: false,
        },
      },
    });
    fireEvent.click(await screen.getByLabelText('module-undo'));

    expect(mockUndo).toHaveBeenCalledOnce();
  });

  it('Calls onRedo when redo is clicked', async () => {
    const mockRedo = vi.fn();

    renderWithStore(<ToolHeader onRedo={mockRedo} />, {
      preloadedState: {
        program: {
          path: '/home/dummy/Documents/test.fl',
          module: simpleModule,
          canRedo: true,
          canUndo: true,
          saved: false,
        },
      },
    });
    fireEvent.click(await screen.getByLabelText('module-redo'));

    expect(mockRedo).toHaveBeenCalledOnce();
  });

  it('Calls onCloseModule when close is clicked', async () => {
    const mockOnCloseModule = vi.fn();

    renderWithStore(<ToolHeader onCloseModule={mockOnCloseModule} />, {
      preloadedState: {
        program: {
          path: '/home/dummy/Documents/test.fl',
          module: simpleModule,
          canRedo: true,
          canUndo: true,
          saved: false,
        },
      },
    });
    fireEvent.click(await screen.getByLabelText('module-close'));

    expect(mockOnCloseModule).toHaveBeenCalledOnce();
  });
});

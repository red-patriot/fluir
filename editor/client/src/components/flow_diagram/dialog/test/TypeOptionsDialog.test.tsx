import { describe, it, expect, afterEach, vi } from 'vitest';
import { cleanup, render, screen } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import '@testing-library/jest-dom';
import TypeOptionsDialog from '@/components/flow_diagram/dialog/TypeOptionsDialog';
import {
  DialogContext,
  DialogActions,
} from '@/components/flow_diagram/dialog/DialogContext';

const mockCloseDialog = vi.fn();
const mockOpenCreateNodeDialog = vi.fn();
const mockOpenTypeOptionsDialog = vi.fn();
const mockOnAccept = vi.fn();

const mockOptions = ['int', 'float', 'string', 'bool'];

const defaultProps = {
  parentID: '1:2',
  where: { x: 100, y: 200 },
  onAccept: mockOnAccept,
  options: mockOptions,
};

function renderDialog(props: typeof defaultProps = defaultProps) {
  const dialogActions: DialogActions = {
    closeDialog: mockCloseDialog,
    openCreateNodeDialog: mockOpenCreateNodeDialog,
    openTypeOptionsDialog: mockOpenTypeOptionsDialog,
  };

  return render(
    <DialogContext.Provider value={dialogActions}>
      <TypeOptionsDialog {...props} />
    </DialogContext.Provider>,
  );
}

describe('TypeOptionsDialog', () => {
  afterEach(() => {
    cleanup();
    vi.clearAllMocks();
  });

  // Rendering

  it('renders all type options when no search text entered', () => {
    renderDialog();
    expect(screen.getByText('int')).toBeInTheDocument();
    expect(screen.getByText('float')).toBeInTheDocument();
    expect(screen.getByText('string')).toBeInTheDocument();
    expect(screen.getByText('bool')).toBeInTheDocument();
  });

  it('renders the search input with placeholder', () => {
    renderDialog();
    expect(
      screen.getByPlaceholderText('Search\u2026'),
    ).toBeInTheDocument();
  });

  // Search filtering

  it('filters options by search text (case-insensitive startsWith)', async () => {
    renderDialog();
    const input = screen.getByPlaceholderText('Search\u2026');
    await userEvent.type(input, 'INT');
    expect(screen.getByText('int')).toBeInTheDocument();
    expect(screen.queryByText('float')).not.toBeInTheDocument();
    expect(screen.queryByText('string')).not.toBeInTheDocument();
    expect(screen.queryByText('bool')).not.toBeInTheDocument();
  });

  it('resets selection index when search text changes', async () => {
    renderDialog();
    const input = screen.getByPlaceholderText('Search\u2026');

    // Arrow down to select first option
    await userEvent.type(input, '{ArrowDown}');
    // The first option should now be selected (blue color)
    const firstOption = screen.getByText('int');
    expect(firstOption.closest('[data-accent-color]')).toHaveAttribute('data-accent-color', 'blue');

    // Type to change search — selection should reset
    await userEvent.type(input, 'f');
    const floatOption = screen.getByText('float');
    expect(floatOption.closest('[data-accent-color]')).toHaveAttribute('data-accent-color', 'gray');
  });

  // Keyboard navigation

  it('ArrowDown moves selection down through options', async () => {
    renderDialog();
    const input = screen.getByPlaceholderText('Search\u2026');

    await userEvent.type(input, '{ArrowDown}');
    expect(screen.getByText('int').closest('[data-accent-color]')).toHaveAttribute(
      'data-accent-color',
      'blue',
    );

    await userEvent.type(input, '{ArrowDown}');
    expect(screen.getByText('float').closest('[data-accent-color]')).toHaveAttribute(
      'data-accent-color',
      'blue',
    );
    expect(screen.getByText('int').closest('[data-accent-color]')).toHaveAttribute(
      'data-accent-color',
      'gray',
    );
  });

  it('ArrowUp moves selection up and stops at 0', async () => {
    renderDialog();
    const input = screen.getByPlaceholderText('Search\u2026');

    // Move down twice, then up twice
    await userEvent.type(input, '{ArrowDown}{ArrowDown}{ArrowUp}');
    expect(screen.getByText('int').closest('[data-accent-color]')).toHaveAttribute(
      'data-accent-color',
      'blue',
    );

    // ArrowUp again should stay at 0
    await userEvent.type(input, '{ArrowUp}');
    expect(screen.getByText('int').closest('[data-accent-color]')).toHaveAttribute(
      'data-accent-color',
      'blue',
    );
  });

  it('ArrowDown stops at last option', async () => {
    renderDialog();
    const input = screen.getByPlaceholderText('Search\u2026');

    // Press ArrowDown more times than there are options
    await userEvent.type(
      input,
      '{ArrowDown}{ArrowDown}{ArrowDown}{ArrowDown}{ArrowDown}{ArrowDown}',
    );
    expect(screen.getByText('bool').closest('[data-accent-color]')).toHaveAttribute(
      'data-accent-color',
      'blue',
    );
    expect(screen.getByText('int').closest('[data-accent-color]')).toHaveAttribute(
      'data-accent-color',
      'gray',
    );
    expect(screen.getByText('float').closest('[data-accent-color]')).toHaveAttribute(
      'data-accent-color',
      'gray',
    );
    expect(screen.getByText('string').closest('[data-accent-color]')).toHaveAttribute(
      'data-accent-color',
      'gray',
    );
  });

  it('Enter on selected option calls onAccept and closeDialog', async () => {
    renderDialog();
    const input = screen.getByPlaceholderText('Search\u2026');

    await userEvent.type(input, '{ArrowDown}{Enter}');
    expect(mockOnAccept).toHaveBeenCalledWith('int');
    expect(mockCloseDialog).toHaveBeenCalledTimes(1);
  });

  // Mouse interaction

  it('hovering an option highlights it', async () => {
    renderDialog();
    const floatOption = screen.getByText('float');

    await userEvent.hover(floatOption);
    expect(floatOption.closest('[data-accent-color]')).toHaveAttribute('data-accent-color', 'blue');
  });

  it('clicking an option calls onAccept and closeDialog', async () => {
    renderDialog();
    const intOption = screen.getByText('int');

    await userEvent.click(intOption);
    expect(mockOnAccept).toHaveBeenCalledWith('int');
    expect(mockCloseDialog).toHaveBeenCalledTimes(1);
  });

  // Scroll area

  it('renders options inside a scroll area with many options', () => {
    const manyOptions = [
      'int',
      'float',
      'string',
      'bool',
      'char',
      'double',
      'long',
      'short',
    ];

    renderDialog({ ...defaultProps, options: manyOptions });

    const scrollViewport = document.querySelector(
      '[data-radix-scroll-area-viewport]',
    );
    expect(scrollViewport).not.toBeNull();

    for (const opt of manyOptions) {
      expect(screen.getByText(opt)).toBeInTheDocument();
    }
  });
});

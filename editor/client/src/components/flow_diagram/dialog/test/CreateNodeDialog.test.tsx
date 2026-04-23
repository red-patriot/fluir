import { describe, it, expect, afterEach, vi } from 'vitest';
import { cleanup, render, screen } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import '@testing-library/jest-dom';
import CreateNodeDialog from '@/components/flow_diagram/dialog/CreateNodeDialog';
import { Completion } from '@/models/intelligence_response';
import { LIMITS } from '@/limits';
import {
  DialogContext,
  DialogActions,
} from '@/components/flow_diagram/dialog/DialogContext';
import {
  ProgramActionsContext,
  ProgramActions,
} from '@/components/reusable/ProgramActionsContext';
import { mockActions } from '@/utility/testProgramActions';

const mockCloseDialog = vi.fn();
const mockOpenCreateNodeDialog = vi.fn();
const mockEditProgram = vi.fn();

const mockOptions: Completion[] = [
  { short_name: '+ binary', kind: 'operator', description: 'Add' },
  { short_name: '- unary', kind: 'operator', description: 'Negate' },
  { short_name: 'int', kind: 'constant', description: 'Integer' },
];

const defaultProps = {
  parentID: '1:2',
  parentLocation: { x: 10, y: 20, z: 0, width: 100, height: 100 },
  clickedLocation: { x: 50, y: 60 },
  where: { x: 100, y: 200 },
  options: mockOptions,
};

function renderDialog(props: typeof defaultProps = defaultProps) {
  const dialogActions: DialogActions = {
    closeDialog: mockCloseDialog,
    openCreateNodeDialog: mockOpenCreateNodeDialog,
    openTypeOptionsDialog: vi.fn(),
  };

  const programActions: ProgramActions = {
    ...mockActions,
    editProgram: mockEditProgram,
  };

  return render(
    <DialogContext.Provider value={dialogActions}>
      <ProgramActionsContext.Provider value={programActions}>
        <CreateNodeDialog {...props} />
      </ProgramActionsContext.Provider>
    </DialogContext.Provider>,
  );
}

describe('CreateNodeDialog', () => {
  afterEach(() => {
    cleanup();
    vi.clearAllMocks();
  });

  // Rendering

  it('renders all options when no search text entered', () => {
    renderDialog();
    expect(screen.getByText('+ binary')).toBeInTheDocument();
    expect(screen.getByText('- unary')).toBeInTheDocument();
    expect(screen.getByText('int')).toBeInTheDocument();
  });

  it('renders the search input with placeholder', () => {
    renderDialog();
    expect(
      screen.getByPlaceholderText('Search…'),
    ).toBeInTheDocument();
  });

  // Search filtering

  it('filters options by search text (case-insensitive startsWith)', async () => {
    renderDialog();
    const input = screen.getByPlaceholderText('Search…');
    await userEvent.type(input, 'INT');
    expect(screen.getByText('int')).toBeInTheDocument();
    expect(screen.queryByText('+ binary')).not.toBeInTheDocument();
    expect(screen.queryByText('- unary')).not.toBeInTheDocument();
  });

  it('resets selection index when search text changes', async () => {
    renderDialog();
    const input = screen.getByPlaceholderText('Search…');

    // Arrow down to select first option
    await userEvent.type(input, '{ArrowDown}');
    // The first option should now be selected (blue color)
    const firstOption = screen.getByText('+ binary');
    expect(firstOption.parentElement!.closest('[data-accent-color]')).toHaveAttribute('data-accent-color', 'blue');

    // Type to change search — selection should reset
    await userEvent.type(input, 'i');
    const intOption = screen.getByText('int');
    expect(intOption.parentElement!.closest('[data-accent-color]')).toHaveAttribute('data-accent-color', 'gray');
  });

  // Keyboard navigation

  it('ArrowDown moves selection down through filtered options', async () => {
    renderDialog();
    const input = screen.getByPlaceholderText('Search…');

    await userEvent.type(input, '{ArrowDown}');
    expect(screen.getByText('+ binary').parentElement!.closest('[data-accent-color]')).toHaveAttribute(
      'data-accent-color',
      'blue',
    );

    await userEvent.type(input, '{ArrowDown}');
    expect(screen.getByText('- unary').parentElement!.closest('[data-accent-color]')).toHaveAttribute(
      'data-accent-color',
      'blue',
    );
    expect(screen.getByText('+ binary').parentElement!.closest('[data-accent-color]')).toHaveAttribute(
      'data-accent-color',
      'gray',
    );
  });

  it('ArrowUp moves selection up and stops at 0', async () => {
    renderDialog();
    const input = screen.getByPlaceholderText('Search…');

    // Move down twice, then up twice
    await userEvent.type(input, '{ArrowDown}{ArrowDown}{ArrowUp}');
    expect(screen.getByText('+ binary').parentElement!.closest('[data-accent-color]')).toHaveAttribute(
      'data-accent-color',
      'blue',
    );

    // ArrowUp again should stay at 0
    await userEvent.type(input, '{ArrowUp}');
    expect(screen.getByText('+ binary').parentElement!.closest('[data-accent-color]')).toHaveAttribute(
      'data-accent-color',
      'blue',
    );
  });

  it('ArrowDown stops at last option', async () => {
    renderDialog();
    const input = screen.getByPlaceholderText('Search…');

    // Press ArrowDown more times than there are options
    await userEvent.type(
      input,
      '{ArrowDown}{ArrowDown}{ArrowDown}{ArrowDown}{ArrowDown}',
    );
    expect(screen.getByText('int').parentElement!.closest('[data-accent-color]')).toHaveAttribute(
      'data-accent-color',
      'blue',
    );
    expect(screen.getByText('+ binary').parentElement!.closest('[data-accent-color]')).toHaveAttribute(
      'data-accent-color',
      'gray',
    );
    expect(screen.getByText('- unary').parentElement!.closest('[data-accent-color]')).toHaveAttribute(
      'data-accent-color',
      'gray',
    );
  });

  it('Enter on a selected option calls editProgram and closeDialog', async () => {
    renderDialog();
    const input = screen.getByPlaceholderText('Search…');

    await userEvent.type(input, '{ArrowDown}{Enter}');
    expect(mockEditProgram).toHaveBeenCalledTimes(1);
    expect(mockCloseDialog).toHaveBeenCalledTimes(1);
  });

  // Mouse interaction

  it('hovering an option highlights it', async () => {
    renderDialog();
    const intOption = screen.getByText('int');

    await userEvent.hover(intOption);
    expect(intOption.parentElement!.closest('[data-accent-color]')).toHaveAttribute('data-accent-color', 'blue');
  });

  it('clicking an option calls editProgram and closeDialog', async () => {
    renderDialog();
    const intOption = screen.getByText('int');

    await userEvent.click(intOption);
    expect(mockEditProgram).toHaveBeenCalledTimes(1);
    expect(mockCloseDialog).toHaveBeenCalledTimes(1);
  });

  // Request construction

  it('builds correct request for an operator completion', async () => {
    renderDialog();
    const option = screen.getByText('+ binary');

    await userEvent.click(option);
    expect(mockEditProgram).toHaveBeenCalledWith({
      discriminator: 'add_node',
      parent: [1, 2],
      new_location: { x: 40, y: 40, z: 1, width: 8, height: 5 },
      params: {
        discriminator: 'operator',
        arity: 'binary',
        op: '+',
      },
    });
  });

  it('builds correct request for a constant completion', async () => {
    renderDialog();
    const option = screen.getByText('int');

    await userEvent.click(option);
    expect(mockEditProgram).toHaveBeenCalledWith({
      discriminator: 'add_node',
      parent: [1, 2],
      new_location: { x: 40, y: 40, z: 1, width: 12, height: 5 },
      params: { discriminator: 'constant', type: 'int' },
    });
  });

  it('builds correct request for a call completion', async () => {
    const callOptions: Completion[] = [
      { short_name: 'myFunc', kind: 'call', description: '' },
    ];

    renderDialog({ ...defaultProps, options: callOptions });
    const option = screen.getByText('myFunc');

    await userEvent.click(option);
    expect(mockEditProgram).toHaveBeenCalledWith({
      discriminator: 'add_node',
      parent: [1, 2],
      new_location: {
        x: 40,
        y: 40,
        z: 1,
        width: LIMITS.call.width.min,
        height: LIMITS.call.height.min,
      },
      params: { discriminator: 'call', target: 'myFunc' },
    });
  });

  // Scroll area

  it('renders options inside a scroll area', () => {
    const manyOptions: Completion[] = [
      { short_name: 'op1 binary', kind: 'operator', description: 'Op1' },
      { short_name: 'op2 binary', kind: 'operator', description: 'Op2' },
      { short_name: 'op3 binary', kind: 'operator', description: 'Op3' },
      { short_name: 'op4 binary', kind: 'operator', description: 'Op4' },
      { short_name: 'op5 binary', kind: 'operator', description: 'Op5' },
      { short_name: 'op6 binary', kind: 'operator', description: 'Op6' },
      { short_name: 'op7 binary', kind: 'operator', description: 'Op7' },
      { short_name: 'op8 binary', kind: 'operator', description: 'Op8' },
    ];

    renderDialog({ ...defaultProps, options: manyOptions });

    const scrollViewport = document.querySelector(
      '[data-radix-scroll-area-viewport]',
    );
    expect(scrollViewport).not.toBeNull();

    for (const opt of manyOptions) {
      expect(screen.getByText(opt.short_name)).toBeInTheDocument();
    }
  });

  it('renders all options when fewer than max visible', () => {
    renderDialog();

    expect(screen.getByText('+ binary')).toBeInTheDocument();
    expect(screen.getByText('- unary')).toBeInTheDocument();
    expect(screen.getByText('int')).toBeInTheDocument();

    const scrollViewport = document.querySelector(
      '[data-radix-scroll-area-viewport]',
    );
    expect(scrollViewport).not.toBeNull();
  });
});

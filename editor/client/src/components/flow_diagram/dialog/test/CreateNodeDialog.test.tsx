import { describe, it, expect, afterEach, vi } from 'vitest';
import { cleanup, render, screen } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import '@testing-library/jest-dom';
import CreateNodeDialog from '@/components/flow_diagram/dialog/CreateNodeDialog';
import { Completion } from '@/models/intelligence_response';

const mockCloseDialog = vi.fn();
const mockEditProgram = vi.fn();

vi.mock('@/components/flow_diagram/dialog/DialogContext', () => ({
  useDialogContext: () => ({ closeDialog: mockCloseDialog }),
}));

vi.mock('@/components/reusable/ProgramActionsContext', () => ({
  useProgramActions: () => ({ editProgram: mockEditProgram }),
}));

const mockOptions: Completion[] = [
  { short_name: '+ binary', kind: 'operator', description: 'Add' },
  { short_name: '- unary', kind: 'operator', description: 'Negate' },
  { short_name: 'int', kind: 'constant', description: 'Integer' },
];

const defaultProps = {
  parentID: '1:2',
  parentLocation: { x: 10, y: 20, z: 0 },
  clickedLocation: { x: 50, y: 60 },
  where: { x: 100, y: 200 },
  options: mockOptions,
};

describe('CreateNodeDialog', () => {
  afterEach(() => {
    cleanup();
    vi.clearAllMocks();
  });

  // Rendering

  it('renders all options when no search text entered', () => {
    render(<CreateNodeDialog {...defaultProps} />);
    expect(screen.getByText('+ binary')).toBeInTheDocument();
    expect(screen.getByText('- unary')).toBeInTheDocument();
    expect(screen.getByText('int')).toBeInTheDocument();
  });

  it('renders the search input with placeholder', () => {
    render(<CreateNodeDialog {...defaultProps} />);
    expect(
      screen.getByPlaceholderText('Search…'),
    ).toBeInTheDocument();
  });

  // Search filtering

  it('filters options by search text (case-insensitive startsWith)', async () => {
    render(<CreateNodeDialog {...defaultProps} />);
    const input = screen.getByPlaceholderText('Search…');
    await userEvent.type(input, 'INT');
    expect(screen.getByText('int')).toBeInTheDocument();
    expect(screen.queryByText('+ binary')).not.toBeInTheDocument();
    expect(screen.queryByText('- unary')).not.toBeInTheDocument();
  });

  it('resets selection index when search text changes', async () => {
    render(<CreateNodeDialog {...defaultProps} />);
    const input = screen.getByPlaceholderText('Search…');

    // Arrow down to select first option
    await userEvent.type(input, '{ArrowDown}');
    // The first option should now be selected (blue color)
    const firstOption = screen.getByText('+ binary');
    expect(firstOption).toHaveAttribute('data-accent-color', 'blue');

    // Type to change search — selection should reset
    await userEvent.type(input, 'i');
    const intOption = screen.getByText('int');
    expect(intOption).toHaveAttribute('data-accent-color', 'gray');
  });

  // Keyboard navigation

  it('ArrowDown moves selection down through filtered options', async () => {
    render(<CreateNodeDialog {...defaultProps} />);
    const input = screen.getByPlaceholderText('Search…');

    await userEvent.type(input, '{ArrowDown}');
    expect(screen.getByText('+ binary')).toHaveAttribute(
      'data-accent-color',
      'blue',
    );

    await userEvent.type(input, '{ArrowDown}');
    expect(screen.getByText('- unary')).toHaveAttribute(
      'data-accent-color',
      'blue',
    );
    expect(screen.getByText('+ binary')).toHaveAttribute(
      'data-accent-color',
      'gray',
    );
  });

  it('ArrowUp moves selection up and stops at 0', async () => {
    render(<CreateNodeDialog {...defaultProps} />);
    const input = screen.getByPlaceholderText('Search…');

    // Move down twice, then up twice
    await userEvent.type(input, '{ArrowDown}{ArrowDown}{ArrowUp}');
    expect(screen.getByText('+ binary')).toHaveAttribute(
      'data-accent-color',
      'blue',
    );

    // ArrowUp again should stay at 0
    await userEvent.type(input, '{ArrowUp}');
    expect(screen.getByText('+ binary')).toHaveAttribute(
      'data-accent-color',
      'blue',
    );
  });

  it('ArrowDown stops at last option', async () => {
    render(<CreateNodeDialog {...defaultProps} />);
    const input = screen.getByPlaceholderText('Search…');

    // Press ArrowDown more times than there are options
    await userEvent.type(
      input,
      '{ArrowDown}{ArrowDown}{ArrowDown}{ArrowDown}{ArrowDown}',
    );
    expect(screen.getByText('int')).toHaveAttribute(
      'data-accent-color',
      'blue',
    );
    expect(screen.getByText('+ binary')).toHaveAttribute(
      'data-accent-color',
      'gray',
    );
    expect(screen.getByText('- unary')).toHaveAttribute(
      'data-accent-color',
      'gray',
    );
  });

  it('Enter on a selected option calls editProgram and closeDialog', async () => {
    render(<CreateNodeDialog {...defaultProps} />);
    const input = screen.getByPlaceholderText('Search…');

    await userEvent.type(input, '{ArrowDown}{Enter}');
    expect(mockEditProgram).toHaveBeenCalledTimes(1);
    expect(mockCloseDialog).toHaveBeenCalledTimes(1);
  });

  // Mouse interaction

  it('hovering an option highlights it', async () => {
    render(<CreateNodeDialog {...defaultProps} />);
    const intOption = screen.getByText('int');

    await userEvent.hover(intOption);
    expect(intOption).toHaveAttribute('data-accent-color', 'blue');
  });

  it('clicking an option calls editProgram and closeDialog', async () => {
    render(<CreateNodeDialog {...defaultProps} />);
    const intOption = screen.getByText('int');

    await userEvent.click(intOption);
    expect(mockEditProgram).toHaveBeenCalledTimes(1);
    expect(mockCloseDialog).toHaveBeenCalledTimes(1);
  });

  // Request construction

  it('builds correct request for an operator completion', async () => {
    render(<CreateNodeDialog {...defaultProps} />);
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
    render(<CreateNodeDialog {...defaultProps} />);
    const option = screen.getByText('int');

    await userEvent.click(option);
    expect(mockEditProgram).toHaveBeenCalledWith({
      discriminator: 'add_node',
      parent: [1, 2],
      new_location: { x: 40, y: 40, z: 1, width: 12, height: 5 },
      params: { discriminator: 'constant', type: 'int' },
    });
  });
});

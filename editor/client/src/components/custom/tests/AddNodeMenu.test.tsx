import { describe, it, expect, beforeEach, vi } from 'vitest';
import { ContextMenu } from 'radix-ui';
import { renderWithStore } from '../../../utility/testStore';
import AddNodeMenu from '../AddNodeMenu';
import userEvent from '@testing-library/user-event';
import '@testing-library/jest-dom';
import { screen } from '@testing-library/react';

describe('AddNodeMenu', () => {
  const editMock = vi.fn();

  beforeEach(async () => {
    renderWithStore(
      <ContextMenu.Root>
        <ContextMenu.Trigger>
          <div data-testid='trigger'></div>
        </ContextMenu.Trigger>
        <AddNodeMenu
          parentID='1:2'
          editProgram={editMock}
          addLocation={{ x: 10, y: -10, z: 1 }}
        />
      </ContextMenu.Root>,
    );
  });

  it('shows on right click', async () => {
    const user = userEvent.setup();

    await user.pointer({
      keys: '[MouseRight]',
      target: screen.getByTestId('trigger'),
    });

    expect(await screen.getByText('Add Node')).toBeInTheDocument();
  });

  it('calls add constant correctly', async () => {
    const user = userEvent.setup();

    await user.pointer({
      keys: '[MouseRight]',
      target: screen.getByTestId('trigger'),
    });

    await user.click(screen.getByText('Float Constant'));

    expect(editMock).toHaveBeenCalledWith({
      discriminator: 'add_node',
      parent: [1, 2],
      new_type: 'Constant',
      new_location: {
        x: 10,
        y: -10,
        z: 1,
        width: 12,
        height: 5,
      },
    });
  });

  it('calls add binary operator correctly', async () => {
    const user = userEvent.setup();

    await user.pointer({
      keys: '[MouseRight]',
      target: screen.getByTestId('trigger'),
    });

    await user.click(screen.getByText('Binary Operator'));

    expect(editMock).toHaveBeenCalledWith({
      discriminator: 'add_node',
      parent: [1, 2],
      new_type: 'BinaryOperator',
      new_location: {
        x: 10,
        y: -10,
        z: 1,
        width: 5,
        height: 5,
      },
    });
  });

  it('calls add unary operator correctly', async () => {
    const user = userEvent.setup();

    await user.pointer({
      keys: '[MouseRight]',
      target: screen.getByTestId('trigger'),
    });

    await user.click(screen.getByText('Unary Operator'));

    expect(editMock).toHaveBeenCalledWith({
      discriminator: 'add_node',
      parent: [1, 2],
      new_type: 'UnaryOperator',
      new_location: {
        x: 10,
        y: -10,
        z: 1,
        width: 5,
        height: 5,
      },
    });
  });
});

import { describe, it, expect, vi } from 'vitest';
import { fireEvent, render, screen } from '@testing-library/react';
import IconButton from '../IconButton';

describe('IconButton', () => {
  it('Displays the correct text', async () => {
    render(<IconButton text='Test Button' />);

    expect(await screen.findAllByText('Test Button')).toHaveLength(1);
  });

  it('Calls onClick when clicked', () => {
    const mockOnClick = vi.fn();

    render(
      <IconButton
        onClick={mockOnClick}
        text='Test Button'
      />,
    );

    fireEvent.click(screen.getByText('Test Button'));

    expect(mockOnClick).toHaveBeenCalled();
  });
});

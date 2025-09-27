import { describe, it, expect, afterEach, vi } from 'vitest';
import editWithChoicePopover, { ChoicePopover } from '../ChoicePopover';
import { cleanup, fireEvent, render, screen } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import '@testing-library/jest-dom';

describe('ChoicePopover', () => {
  afterEach(vi.resetAllMocks);

  it('displays the given options', () => {
    const choices = ['0', '1', '2'];

    render(
      <ChoicePopover
        id='0'
        current=''
        choices={choices}
        onClick={() => {}}
      />,
    );

    for (const choice of choices) {
      expect(screen.getByText(choice)).toBeVisible();
    }
  });

  it('calls onClick with the selected option', () => {
    const choices = ['0', '1', '2'];
    const onClickMock = vi.fn();

    render(
      <ChoicePopover
        id='0'
        current='0'
        choices={choices}
        onClick={onClickMock}
      />,
    );

    for (const choice of choices) {
      fireEvent.click(screen.getByLabelText(`choice-${choice}`));

      expect(onClickMock).toHaveBeenCalledWith(choice);
    }
  });

  it('calls onDone hook when clicked', () => {
    const choices = ['0', '1', '2'];
    const onDoneMock = vi.fn();
    const onClickMock = vi.fn();

    const renderer = editWithChoicePopover(onClickMock, choices);

    render(renderer('0', '0', onDoneMock()));

    fireEvent.click(screen.getByLabelText('choice-1'));
    expect(onDoneMock).toHaveBeenCalledOnce();
  });
});

import { describe, it, expect, vi } from 'vitest';
import { render, screen } from '@testing-library/react';
import { ValueDisplay } from '@/components/flow_diagram/common/ValueDisplay';
import { userEvent } from '@testing-library/user-event';

describe('ValueDisplay', () => {
  it('renders the value correctly', () => {
    render(
      <ValueDisplay
        fullID='1'
        value='12345'
      />,
    );

    expect(screen.getByText('12345')).toBeInTheDocument();
  });

  it('Runs the validation function on enter', async () => {
    const validateMock = vi.fn();

    render(
      <ValueDisplay
        fullID='1'
        value='123'
        editable
        validate={validateMock}
      />,
    );

    await userEvent.click(screen.getByLabelText('1-value-display'));
    await userEvent.type(screen.getByLabelText('1-input-field'), '45{enter}');

    expect(validateMock).toHaveBeenCalledWith('12345');
  });

  it('Runs the success function on enter', async () => {
    const succeedMock = vi.fn();
    const justSucceed = () => true;

    render(
      <ValueDisplay
        fullID='1'
        value='123'
        editable
        validate={justSucceed}
        onValidateSucceed={succeedMock}
      />,
    );

    await userEvent.click(screen.getByLabelText('1-value-display'));
    await userEvent.type(screen.getByLabelText('1-input-field'), '45{enter}');

    expect(succeedMock).toHaveBeenCalledWith('12345');
  });

  it('Runs the failure function on enter', async () => {
    const succeedMock = vi.fn();
    const failMock = vi.fn();
    const justFail = () => false;

    render(
      <ValueDisplay
        fullID='1'
        value='12345'
        editable
        validate={justFail}
        onValidateSucceed={succeedMock}
        onValidateFail={failMock}
      />,
    );

    await userEvent.click(screen.getByLabelText('1-value-display'));
    await userEvent.type(screen.getByLabelText('1-input-field'), 'ab{enter}');

    expect(succeedMock).not.toHaveBeenCalled();
    expect(failMock).toHaveBeenCalled();
  });
});

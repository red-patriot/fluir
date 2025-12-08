import { describe, it, expect, vi } from 'vitest';
import { render, screen } from '@testing-library/react';
import { ValueDisplay } from '@/components/flow_diagram/common/ValueDisplay';
import InputField from '@/components/flow_diagram/common/InputField';
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
    const createInputField = (
      id: string,
      value: string,
      onDone: () => void,
    ) => (
      <InputField
        id={id}
        initialValue={value}
        onDone={onDone}
        validate={validateMock}
      />
    );

    render(
      <ValueDisplay
        fullID='1'
        value='123'
        renderEdit={createInputField}
      ></ValueDisplay>,
    );

    await userEvent.click(screen.getByLabelText('1-value-display'));
    await userEvent.type(screen.getByLabelText('1-input-field'), '45{enter}');

    expect(validateMock).toHaveBeenCalledWith('12345');
  });

  it('Runs the success function on enter', async () => {
    const succeedMock = vi.fn();
    const justSucceed = () => true;
    const createInputField = (
      id: string,
      value: string,
      onDone: () => void,
    ) => (
      <InputField
        id={id}
        initialValue={value}
        onDone={onDone}
        validate={justSucceed}
        onValidateSucceed={succeedMock}
      />
    );

    render(
      <ValueDisplay
        fullID='1'
        value='123'
        renderEdit={createInputField}
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
    const createInputField = (
      id: string,
      value: string,
      onDone: () => void,
    ) => (
      <InputField
        id={id}
        initialValue={value}
        onDone={onDone}
        validate={justFail}
        onValidateSucceed={succeedMock}
        onValidateFail={failMock}
      />
    );

    render(
      <ValueDisplay
        fullID='1'
        value='12345'
        renderEdit={createInputField}
      />,
    );

    await userEvent.click(screen.getByLabelText('1-value-display'));
    await userEvent.type(screen.getByLabelText('1-input-field'), 'ab{enter}');

    expect(succeedMock).not.toHaveBeenCalled();
    expect(failMock).toHaveBeenCalled();
  });
});

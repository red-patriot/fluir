import { describe, it, expect, afterEach, vi } from 'vitest';
import InputField from '../InputField';
import { cleanup, render, screen } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import '@testing-library/jest-dom';

describe('InputField', () => {
  afterEach(cleanup);

  it('calls validator on enter', async () => {
    const validate = vi.fn();
    validate.mockReturnValue(true);

    render(
      <InputField
        id={'1:3:6'}
        validate={validate}
      />,
    );

    await userEvent.type(
      screen.getByLabelText('1:3:6-input-field'),
      'hello{enter}',
    );

    expect(validate).toHaveBeenCalledWith('hello');
  });

  it('calls calls succees handler if validate succeeds', async () => {
    const validate = vi.fn();
    validate.mockReturnValue(true);
    const onValidateSucceed = vi.fn();
    const onValidateFail = vi.fn();

    render(
      <InputField
        id={'1:3:6'}
        validate={validate}
        onValidateSucceed={onValidateSucceed}
        onValidateFail={onValidateFail}
      />,
    );

    await userEvent.type(
      screen.getByLabelText('1:3:6-input-field'),
      'hello{enter}',
    );

    expect(validate).toHaveBeenCalledWith('hello');
    expect(onValidateSucceed).toHaveBeenCalledWith('hello');
    expect(onValidateFail).not.toHaveBeenCalled();
  });

  it('calls calls fail handler if validate fails', async () => {
    const validate = vi.fn();
    validate.mockReturnValue(false);
    const onValidateSucceed = vi.fn();
    const onValidateFail = vi.fn();

    render(
      <InputField
        id={'1:3:6'}
        validate={validate}
        onValidateSucceed={onValidateSucceed}
        onValidateFail={onValidateFail}
      />,
    );

    await userEvent.type(
      screen.getByLabelText('1:3:6-input-field'),
      'hello{enter}',
    );

    expect(validate).toHaveBeenCalledWith('hello');
    expect(onValidateSucceed).not.toHaveBeenCalled();
    expect(onValidateFail).toHaveBeenCalled();
  });

  it('calls calls succees handler if no validator', async () => {
    const onValidateSucceed = vi.fn();
    const onValidateFail = vi.fn();

    render(
      <InputField
        id={'1:3:6'}
        onValidateSucceed={onValidateSucceed}
        onValidateFail={onValidateFail}
      />,
    );

    await userEvent.type(
      screen.getByLabelText('1:3:6-input-field'),
      'hello{enter}',
    );

    expect(onValidateSucceed).toHaveBeenCalledWith('hello');
    expect(onValidateFail).not.toHaveBeenCalled();
  });

  it('calls cancel handler', async () => {
    const onCancel = vi.fn();

    render(
      <InputField
        id={'1:3:6'}
        onCancel={onCancel}
      />,
    );

    await userEvent.type(
      screen.getByLabelText('1:3:6-input-field'),
      'hello{escape}',
    );

    expect(onCancel).toHaveBeenCalled();
  });
});

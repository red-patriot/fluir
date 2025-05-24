import { describe, it, expect, beforeEach, afterEach } from 'vitest';
import ConstantElement from '../ConstantElement';
import { Constant } from '../../../models/fluir_module';
import { renderWithStore } from '../../../utility/testStore';
import { cleanup, fireEvent, screen } from '@testing-library/react';
import userEvent from '@testing-library/user-event';
import { mockActions } from '../../../utility/testProgramActions';
import '@testing-library/jest-dom';
import { UpdateConstantEditRequest } from '../../../models/edit_request';

describe('ConstantElement', () => {
  const data: Constant = {
    discriminator: 'constant',
    id: 1,
    location: { x: 3, y: 6, z: 12, width: 14, height: 5 },
    flType: 'FLOATING_POINT',
    value: '4.3213',
  };
  it('Shows the correct constant value', async () => {
    renderWithStore(<ConstantElement constant={data} />);

    expect(await screen.getByText('4.3213')).toBeVisible();
  });

  it('Shows at the correct full ID', async () => {
    const expected = 'constant-4:1';
    renderWithStore(
      <ConstantElement
        constant={data}
        parentId={'4'}
      />,
    );

    expect(await screen.findByLabelText(expected)).toBeVisible();
  });

  it('Shows at the correct location', async () => {
    const expectedX = '9px';
    const expectedY = '18px';
    renderWithStore(<ConstantElement constant={data} />, {
      preloadedState: {
        program: {
          zoom: 10.0,
        },
      },
    });

    const x = await screen.getByLabelText('constant-1').style.left;
    const y = await screen.getByLabelText('constant-1').style.top;

    expect(x).toStrictEqual(expectedX);
    expect(y).toStrictEqual(expectedY);
  });
  it('Changes to edit mode', async () => {
    renderWithStore(<ConstantElement constant={data} />);

    await fireEvent.click(screen.getByLabelText('constant-1-view'));

    expect(screen.getByLabelText('constant-1-edit')).toBeInTheDocument();
  });

  describe('Edit Operations', () => {
    beforeEach(async () => {
      renderWithStore(<ConstantElement constant={data} />);

      await fireEvent.click(screen.getByLabelText('constant-1-view'));
    });
    afterEach(cleanup);

    it('Can be cancelled', async () => {
      await userEvent.type(
        screen.getByLabelText('constant-1-edit'),
        '1.3453{escape}',
      );

      expect(screen.getByLabelText('constant-1-view')).toBeInTheDocument();
    });

    it('Can be committed', async () => {
      const expected: UpdateConstantEditRequest = {
        discriminator: 'update_constant',
        target: [1],
        value: '4.32133453',
      };

      await userEvent.type(
        screen.getByLabelText('constant-1-edit'),
        '3453{enter}',
      );

      expect(screen.getByLabelText('constant-1-view')).toBeInTheDocument();
      expect(mockActions.editProgram).toHaveBeenCalledWith(expected);
    });
  });
});

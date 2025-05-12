import { describe, it, expect } from 'vitest';
import OpBinaryElement from '../OpBinaryElement';
import { BinaryOp } from '../../../models/fluir_module';
import { renderWithStore } from '../../../utility/testStore';
import { screen } from '@testing-library/react';
import '@testing-library/jest-dom';

describe('OpBinaryElement', () => {
  const data: BinaryOp = {
    _t: 'binary',
    id: 2,
    location: { x: 9, y: 18, z: 3, width: 14, height: 5 },
    op: '-',
    lhs: 3,
    rhs: 7,
  };
  it('Shows the correct operator', async () => {
    renderWithStore(<OpBinaryElement binary={data} />);

    expect(await screen.getByText('-')).toBeVisible();
  });

  it('Shows at the correct full ID', async () => {
    const expected = 'binary-4:2';
    renderWithStore(
      <OpBinaryElement
        binary={data}
        parentId={'4'}
      />,
    );

    expect(await screen.findByLabelText(expected)).toBeVisible();
  });

  it('Shows at the correct location', async () => {
    const expectedX = '27px';
    const expectedY = '54px';
    renderWithStore(<OpBinaryElement binary={data} />, {
      preloadedState: {
        program: {
          zoom: 10.0,
        },
      },
    });

    const x = await screen.getByLabelText('binary-2').style.left;
    const y = await screen.getByLabelText('binary-2').style.top;

    expect(x).toStrictEqual(expectedX);
    expect(y).toStrictEqual(expectedY);
  });
});

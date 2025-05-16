import { describe, it, expect } from 'vitest';
import OpUnaryElement from '../OpUnaryElement';
import { UnaryOp } from '../../../models/fluir_module';
import { renderWithStore } from '../../../utility/testStore';
import { screen } from '@testing-library/react';
import '@testing-library/jest-dom';

describe('OpUnaryElement', () => {
  const data: UnaryOp = {
    discriminator: 'unary',
    id: 1,
    location: { x: 1, y: 3, z: 3, width: 14, height: 5 },
    op: '*',
    lhs: 3,
  };
  it('Shows the correct operator', async () => {
    renderWithStore(<OpUnaryElement unary={data} />);

    expect(await screen.getByText('*')).toBeVisible();
  });

    it('Shows at the correct full ID', async () => {
      const expected = 'unary-12:5:8:1';
      renderWithStore(
        <OpUnaryElement
          unary={data}
          parentId={'12:5:8'}
        />,
      );

      expect(await screen.findByLabelText(expected)).toBeVisible();
    });

  it('Shows at the correct location', async () => {
    const expectedX = '3px';
    const expectedY = '9px';
    renderWithStore(<OpUnaryElement unary={data} />, {
      preloadedState: {
        program: {
          zoom: 10.0,
        },
      },
    });

    const x = await screen.getByLabelText('unary-1').style.left;
    const y = await screen.getByLabelText('unary-1').style.top;

    expect(x).toStrictEqual(expectedX);
    expect(y).toStrictEqual(expectedY);
  });
});

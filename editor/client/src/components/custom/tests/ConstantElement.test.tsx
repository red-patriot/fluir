import { describe, it, expect } from 'vitest';
import ConstantElement from '../ConstantElement';
import { Constant } from '../../../models/fluir_module';
import { renderWithStore } from '../../../utility/testStore';
import { screen } from '@testing-library/react';
import '@testing-library/jest-dom';

describe('ConstantElement', () => {
  const data: Constant = {
    _t: 'constant',
    id: 1,
    location: { x: 3, y: 6, z: 12, width: 14, height: 5 },
    value: 4.3213,
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
});

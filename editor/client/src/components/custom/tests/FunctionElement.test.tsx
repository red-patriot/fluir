import { describe, it, expect } from 'vitest';
import FunctionElement from '../FunctionElement';
import { FunctionDecl } from '../../../models/fluir_module';
import { renderWithStore } from '../../../utility/testStore';
import { screen } from '@testing-library/react';
import '@testing-library/jest-dom';

describe('FunctionElement', () => {
  const data: FunctionDecl = {
    _t: 'function',
    id: 7,
    location: { x: 9, y: 18, z: 3, width: 140, height: 500 },
    name: 'testFunc',
    body: [],
  };
  it('Shows the correct name value', async () => {
    renderWithStore(<FunctionElement decl={data} />);

    expect(await screen.getByText('testFunc')).toBeVisible();
  });

  it('Shows at the correct location', async () => {
    const expectedX = '27px';
    const expectedY = '54px';
    renderWithStore(<FunctionElement decl={data} />, {
      preloadedState: {
        program: {
          zoom: 10.0,
        },
      },
    });

    const x = await screen.getByLabelText('func-testFunc-7').style.left;
    const y = await screen.getByLabelText('func-testFunc-7').style.top;

    expect(x).toStrictEqual(expectedX);
    expect(y).toStrictEqual(expectedY);
  });

  it('Shows the correct size', async () => {
    const expectedW = '420px';
    const expectedH = '1500px';
    renderWithStore(<FunctionElement decl={data} />, {
      preloadedState: {
        program: {
          zoom: 10.0,
        },
      },
    });

    const w = await screen.getByLabelText('func-testFunc-7').style.width;
    const h = await screen.getByLabelText('func-testFunc-7').style.height;

    expect(w).toStrictEqual(expectedW);
    expect(h).toStrictEqual(expectedH);
  });
});

import { describe, it, expect, afterEach } from 'vitest';
import ModuleViewWindow from '../ModuleViewWindow';
import { cleanup, screen } from '@testing-library/react';
import { renderWithStore } from '../../../utility/testStore';
import '@testing-library/jest-dom';
import FluirModule from '../../../models/fluir_module';

describe('ModuleViewElement', () => {
  afterEach(cleanup);

  const program: FluirModule = {
    declarations: [
      {
        _t: 'function',
        id: 1,
        name: 'main',
        location: { x: 1, y: 2, z: 1, width: 100, height: 100 },
        body: [],
      },
      {
        _t: 'function',
        id: 2,
        name: 'foo',
        location: { x: 1, y: 2, z: 1, width: 100, height: 100 },
        body: [],
      },
    ],
  };

  it('shows loading if there is no module', async () => {
    renderWithStore(<ModuleViewWindow />);

    expect(await screen.getByText('Loading...')).toBeInTheDocument();
  });

  it('shows multiple functions', async () => {
    renderWithStore(<ModuleViewWindow />, {
      preloadedState: {
        program: {
          module: program,
        },
      },
    });

    expect(screen.getByLabelText('func-main-1')).toBeInTheDocument();
    expect(screen.getByLabelText('func-foo-2')).toBeInTheDocument();
  });
});

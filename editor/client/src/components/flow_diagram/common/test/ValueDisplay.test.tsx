import { describe, it, expect } from 'vitest';
import { render, screen } from '@testing-library/react';
import { ValueDisplay } from '@/components/flow_diagram/common/ValueDisplay';

describe('ValueDisplay', () => {
  it('renders the value correctly', () => {
    render(<ValueDisplay value='12345' />);

    expect(screen.getByText('12345')).toBeInTheDocument();
  });
});

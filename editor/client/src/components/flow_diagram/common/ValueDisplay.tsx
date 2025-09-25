import { Box, Code } from '@radix-ui/themes';

interface ValueDisplayProps {
  value?: string;
}

export function ValueDisplay({ value }: ValueDisplayProps) {
  return (
    <Code
      color='gray'
      variant='solid'
      size='2'
      className='ml-0.25 grow'
    >
      {value}
    </Code>
  );
}

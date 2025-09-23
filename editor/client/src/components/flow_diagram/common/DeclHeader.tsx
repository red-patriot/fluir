import React from 'react';

import { Badge, Box, Code, Flex } from '@radix-ui/themes';

interface DeclHeaderProps {
  name: string;
  variant?: 'solid' | 'ghost';
}

export default function DeclHeader({
  name,
  children,
}: React.PropsWithChildren<DeclHeaderProps>) {
  return (
    <Flex
      direction='row'
      align='center'
    >
      <Badge
        variant='outline'
        className='grow'
      >
        <Code
          color='blue'
          variant='soft'
          size='1'
        >
          {name}
        </Code>
        <Box className='grow' />
        {children}
      </Badge>
    </Flex>
  );
}

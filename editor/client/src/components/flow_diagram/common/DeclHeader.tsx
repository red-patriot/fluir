import React from 'react';
import { amber } from '@radix-ui/colors';
import { Badge, Code, Flex } from '@radix-ui/themes';

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
        variant='solid'
        className='grow'
        style={{
          background: amber.amber8,
        }}
      >
        <Code
          color='gray'
          variant='solid'
          size='1'
          className='shrink overflow-ellipsis'
        >
          {name}
        </Code>
        {children}
      </Badge>
    </Flex>
  );
}

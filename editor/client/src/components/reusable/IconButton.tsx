import React from 'react';
import { Button, ButtonProps } from '@radix-ui/themes';

interface IconButtonProps extends ButtonProps {
  onClick?: () => void;
  text?: string;
  icon?: React.ReactElement;
}

export default function IconButton({
  onClick,
  text,
  icon,
  ...props
}: IconButtonProps) {
  return (
    <Button
      onClick={onClick}
      {...props}
    >
      {icon && icon}
      {text && text}
    </Button>
  );
}

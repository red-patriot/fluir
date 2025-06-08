import React from 'react';
import {
  FontAwesomeIcon,
  FontAwesomeIconProps,
} from '@fortawesome/react-fontawesome';

interface IconButtonProps
  extends React.ButtonHTMLAttributes<HTMLButtonElement> {
  onClick?: () => void;
  iconProps?: FontAwesomeIconProps;
  text?: string;
}

export default function IconButton({
  onClick,
  iconProps,
  text,
  ...props
}: IconButtonProps) {
  return (
    <button
      onClick={onClick && onClick}
      className={`border text-[1em] font-medium
           transition-[border-color]
           duration-[0.25s] px-[0.6em] py-[0.4em]
           border-solid border-transparent rounded-lg
           ${
             props.disabled
               ? 'bg-[#3a3a3a] cursor-not-allowed'
               : 'bg-[#1a1a1a] hover:border-[#646cff]'
           }
           m-px`}
      {...props}
    >
      {iconProps && <FontAwesomeIcon {...iconProps} />}
      {text && text}
    </button>
  );
}

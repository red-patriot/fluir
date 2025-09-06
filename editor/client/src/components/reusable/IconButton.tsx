import React from "react";
import { IconProps } from "@radix-ui/react-icons/dist/types";

interface IconButtonProps extends React.ButtonHTMLAttributes<HTMLButtonElement> {
  onClick?: () => void;
  text?: string;
  icon?:  React.ReactElement
}

export default function IconButton ({onClick, text, icon, ...props}: IconButtonProps){
  return (
    <button
    onClick={onClick}
    {...props}>
      {icon && icon}
      {text && text}
    </button>
  )

}
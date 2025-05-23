import {
  FontAwesomeIcon,
  FontAwesomeIconProps,
} from '@fortawesome/react-fontawesome';

interface IconButtonProps {
  onClick?: () => void;
  iconProps?: FontAwesomeIconProps;
  text?: string;
  label?: string;
}

export default function IconButton({
  onClick,
  iconProps,
  text,
  label,
}: IconButtonProps) {
  return (
    <button
      aria-label={label}
      onClick={onClick && onClick}
      className='border text-[1em] font-medium bg-[#1a1a1a]
                 cursor-pointer transition-[border-color]
                 duration-[0.25s] px-[0.6em] py-[0.4em]
                 border-solid border-transparent
                 hover:border-[#646cff] m-px'
    >
      {iconProps && <FontAwesomeIcon {...iconProps} />}
      {text && text}
    </button>
  );
}

import {
  FontAwesomeIcon,
  FontAwesomeIconProps,
} from '@fortawesome/react-fontawesome';

interface IconButtonProps {
  onClick?: () => void;
  iconProps?: FontAwesomeIconProps;
  text?: string;
}

export default function IconButton({
  onClick,
  iconProps,
  text,
}: IconButtonProps) {
  return (
    <button
      onClick={onClick && onClick}
      className='border text-[1em] font-medium bg-[#1a1a1a]
                 cursor-pointer transition-[border-color]
                 duration-[0.25s] px-[1.2em] py-[0.6em] rounded-lg
                 border-solid border-transparent
                 hover:border-[#646cff] m-px'
    >
      {iconProps && <FontAwesomeIcon {...iconProps} />}
      {text && text}
    </button>
  );
}

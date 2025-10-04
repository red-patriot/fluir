import fluirLogo from '../../assets/fluir-logo.svg';
import IconButton from '@/components/reusable/IconButton';
import { FilePlusIcon, FileIcon } from '@radix-ui/react-icons';
import { Flex } from '@radix-ui/themes';

interface HomeProps {
  onNewFile?: () => void;
  onOpenFile?: () => void;
}

export default function Home({ onNewFile, onOpenFile }: HomeProps) {
  return (
    <Flex
      justify='center'
      align='center'
      className='h-lvh'
    >
      <img
        src={fluirLogo}
        className='logo size-1/3 m-5'
      />
      <Flex
        direction='column'
        gap='3'
      >
        <IconButton
          size='3'
          aria-label='new-file-button'
          onClick={onNewFile}
          variant='surface'
          icon={<FilePlusIcon />}
          text='New File'
        />
        <IconButton
          size='3'
          aria-label='open-file-button'
          onClick={onOpenFile}
          variant='surface'
          icon={<FileIcon />}
          text='Open File'
        />
      </Flex>
    </Flex>
  );
}

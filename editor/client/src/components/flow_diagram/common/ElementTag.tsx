import { Flex, Box } from '@radix-ui/themes';

type Props = { name: string };

export default function ElementTag({ name }: Props) {
  return (
    <Flex
      direction="column"
      height="100%"
    >
      <Box className="grow" />
      <p className="text-[6px] font-mono align-text-bottom">{name}</p>
    </Flex>
  );
}

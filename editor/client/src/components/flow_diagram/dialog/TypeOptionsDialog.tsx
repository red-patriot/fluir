import { Badge } from '@radix-ui/themes';
import { TypeDialogOptions } from './DialogContext';
import CoreDialog, { OptionProps } from './CoreDialog';

interface TypeOptionsDialogProps extends TypeDialogOptions {
  options: string[];
}

export default function TypeOptionsDialog({
  where,
  onAccept,
  options,
}: TypeOptionsDialogProps) {
  const renderOption = ({ data, highlighted }: OptionProps<string>) => (
    <Badge
      color={highlighted ? 'blue' : 'gray'}
      className="cursor-pointer w-full"
    >
      {data}
    </Badge>
  );

  return (
    <CoreDialog
      where={where}
      options={options}
      optionToString={(o) => o}
      onSelect={onAccept}
      renderOption={renderOption}
    />
  );
}

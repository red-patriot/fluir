import { Code } from '@radix-ui/themes';
import { useState } from 'react';
import InputField from '@/components/flow_diagram/common/InputField';

interface ValueDisplayProps {
  fullID: string;
  value?: string;
  editable?: boolean;
  validate?: (text: string) => boolean;
  onValidateSucceed?: (text: string) => void;
  onValidateFail?: () => void;
  onCancel?: () => void;
}

export function ValueDisplay({
  fullID,
  value,
  editable = false,
  validate,
  onValidateSucceed,
  onValidateFail,
  onCancel,
}: ValueDisplayProps) {
  const [isEditing, setIsEditing] = useState(false);

  const startEditing = () => {
    if (editable) {
      setIsEditing(true);
    }
  };

  return (
    <Code
      color='gray'
      variant='solid'
      size='2'
      className='grow ml-0.25 overflow-hidden text-ellipsis whitespace-nowrap'
    >
      {isEditing ? (
        <InputField
          id={fullID}
          initialValue={value}
          validate={validate}
          onValidateSucceed={(text: string) => {
            onValidateSucceed && onValidateSucceed(text);
            setIsEditing(false);
          }}
          onValidateFail={() => {
            onValidateFail && onValidateFail();
            setIsEditing(false);
          }}
          onCancel={() => {
            onCancel && onCancel();
            setIsEditing(false);
          }}
        />
      ) : (
        <span
          aria-label={`${fullID}-value-display`}
          onClick={startEditing}
        >
          {value}
        </span>
      )}
    </Code>
  );

  //   return (
  //     <Code
  //       inputMode='decimal'
  //       color='gray'
  //       variant='solid'
  //       size='2'
  //       className='ml-0.25 grow ellipsis'
  //     >
  //       {value}
  //     </Code>
  //   );
}

import React, { useState, useRef, useEffect } from 'react';

interface InputFieldProps extends React.InputHTMLAttributes<HTMLInputElement> {
  id: string;
  initialValue?: string;
  validate?: (text: string) => boolean;
  onValidateSucceed?: (text: string) => void;
  onValidateFail?: () => void;
  onCancel?: () => void;
}

export default function InputField({
  id,
  initialValue = '',
  validate,
  onValidateSucceed,
  onValidateFail,
  onCancel,
  ...props
}: InputFieldProps) {
  const [tempText, setTempText] = useState(initialValue);
  const inputRef = useRef<HTMLInputElement>(null);

  const handleKeyDown = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter') {
      if (validate && !validate(tempText)) {
        onValidateFail && onValidateFail();
      } else {
        onValidateSucceed && onValidateSucceed(tempText);
      }
    } else if (e.key === 'Escape') {
      onCancel && onCancel();
    }
  };

  useEffect(() => {
    if (inputRef.current) {
      inputRef.current.focus();
      inputRef.current.select();
    }
  }, []);

  return (
    <input
      aria-label={`${id}-input-field`}
      ref={inputRef}
      value={tempText}
      onChange={(e) => setTempText(e.target.value)}
      onKeyDown={handleKeyDown}
      onBlur={onCancel}
      {...props}
    />
  );
}

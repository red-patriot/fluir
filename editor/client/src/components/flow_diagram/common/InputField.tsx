import React, { useState, useRef, useEffect } from 'react';

interface InputFieldProps extends React.InputHTMLAttributes<HTMLInputElement> {
  id: string;
  initialValue?: string;
  onDone?: () => void;
  validate?: (text: string) => boolean;
  onValidateSucceed?: (text: string) => void;
  onValidateFail?: () => void;
  onCancel?: () => void;
}

export default function InputField({
  id,
  initialValue = '',
  onDone = () => {},
  validate,
  onValidateSucceed,
  onValidateFail,
  onCancel,
  ...props
}: InputFieldProps) {
  const [tempText, setTempText] = useState(initialValue);
  const inputRef = useRef<HTMLInputElement>(null);

  const submit = () => {
    if (validate && !validate(tempText)) {
      onValidateFail && onValidateFail();
    } else {
      onValidateSucceed && onValidateSucceed(tempText);
    }
    onDone();
  };

  const cancel = () => {
    onCancel && onCancel();
    onDone();
  };

  const handleKeyDown = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter') {
      submit();
    } else if (e.key === 'Escape') {
      cancel();
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
      onSubmit={submit}
      onBlur={cancel}
      className='w-full h-full focus:outline-none'
      {...props}
    />
  );
}

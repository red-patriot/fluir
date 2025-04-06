import React, { useState, useRef } from "react";
import Diagram from "./Diagram";

const DisplayFluir: React.FC = () => {
  const fileInputRef = useRef<HTMLInputElement>(null);
  const [fileContent, setFileContent] = useState<string>("");
  const [fileName, setFileName] = useState<string>("");

  const onButtonClick = () => {
    fileInputRef.current?.click();
  };

  const onFileChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target?.files?.[0];

    if (file) {
      console.log(`Selected ${file.name}`);
      setFileName(file.name);

      const reader = new FileReader();
      reader.onload = (e) => {
        const content = e.target?.result as string;
        console.log(content);
        setFileContent(content);
      };
      reader.readAsText(file);
    }
  };

  return (
    <div className="flex flex-col">
      <div className="flex flex-row space-x-5">
        <h1 className="align-top">Fluir</h1>
        <button onClick={onButtonClick}>Open File</button>
        <input
          type="file"
          accept=".fl"
          ref={fileInputRef}
          style={{ display: "none" }}
          onChange={onFileChange}
        />
      </div>
      {fileContent && <Diagram contents={fileContent} name={fileName} />}
    </div>
  );
};

export default DisplayFluir;

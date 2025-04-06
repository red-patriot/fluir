import { XMLParser } from "fast-xml-parser";
import toProgram from "./functions/toProgram";
import ViewWindow from "./components/ViewWindow";

interface DiagramProps {
  contents: string;
  name: string;
}

const Diagram = ({ contents, name }: DiagramProps) => {
  const parser = new XMLParser({ ignoreAttributes: false });
  const raw = parser.parse(contents);
  console.log(raw);
  const program = toProgram(raw);

  return (
    <div className="h-screen">
      <h2 className="text-lg font-bold mb-2">{`"${name}" Contents:`}</h2>
      <ViewWindow data={program} />
    </div>
  );
};

export default Diagram;

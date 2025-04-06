import { XMLParser } from "fast-xml-parser";
import Fluir from "./models/fluir.d";
import ViewWindow from "./components/ViewWindow";

interface DiagramProps {
  contents: string;
  name: string;
}

const Diagram = ({ contents, name }: DiagramProps) => {
  const parser = new XMLParser({ ignoreAttributes: false });
  const obj = parser.parse(contents) as Fluir;

  return (
    <div className="h-screen">
      <h2 className="text-lg font-bold mb-2">{`"${name}" Contents:`}</h2>
      <ViewWindow data={obj} />
    </div>
  );
};

export default Diagram;

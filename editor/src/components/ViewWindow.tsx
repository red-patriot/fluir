import Fluir from "../models/fluir.d";

interface ViewWindowProps {
  data: Fluir;
}

const ViewWindow: React.FC<ViewWindowProps> = ({ data }) => {
  const containerHeight = 500;
  const constants = data.fluir["fl:constant"];

  console.log(data);

  return (
    <div className="relative w-[500px] h-[500px] border border-gray-300 bg-stone-700 overflow-hidden">
      {constants.map((constant, index) => {
        const x = constant["@_x"];
        const y = constant["@_y"];
        const w = constant["@_w"];
        const h = constant["@_h"];
        const id = constant["@_id"];

        return (
          <div
            key={index}
            className="absolute bg-blue-400"
            style={{
              left: `${x}px`,
              top: `${containerHeight - (y + h)}px`,
              width: `${w}px`,
              height: `${h}px`,
              border: "1px solid black",
              boxSizing: "border-box",
            }}
            title={`ID: ${id}`}
          ></div>
        );
      })}
    </div>
  );
};

export default ViewWindow;

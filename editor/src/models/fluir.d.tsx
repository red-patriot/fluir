import FLConstant from "./constant.d";

type Fluir = {
  fluir: {
    "fl:constant": FLConstant | FLConstant[];
    "@_xmlns:fl": string;
  };
};

export default Fluir;

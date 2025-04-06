import FLValue from "./value.d";

type FLConstant = {
  "fl:value": FLValue;
  "@_x": string;
  "@_y": string;
  "@_z"?: string;
  "@_w": string;
  "@_h": string;
  "@_id"?: string;
};

export default FLConstant;

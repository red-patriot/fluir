import FLValue from "./value.d";

type FLConstant = {
  "fl:value": FLValue;
  "@_x": number;
  "@_y": number;
  "@_z"?: number;
  "@_w": number;
  "@_h": number;
  "@_id"?: number;
};

export default FLConstant;

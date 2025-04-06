import FLConstant from "../models/constant.d";
import FLValue from "../models/value.d";
import Fluir from "../models/fluir.d";
import FLExpression from "../models/expression.d";

export const toValue = (data: any): FLValue => {
  return {
    literal: parseFloat(data["#text"]),
    "@_type": data["@_type"],
  };
};

export const toConstant = (data: any): FLConstant => {
  return {
    "fl:value": toValue(data["fl:value"]),
    "@_id": parseInt(data["@_id"]),
    "@_x": parseInt(data["@_x"]),
    "@_y": parseInt(data["@_y"]),
    "@_z": parseInt(data["@_z"]),
    "@_w": parseInt(data["@_w"]),
    "@_h": parseInt(data["@_h"]),
  };
};

export const toExpression = (data: any): FLExpression => {
  return toConstant(data);
};

const toProgram = (data: any): Fluir => {
  return {
    fluir: { "fl:constant": [toConstant(data["fluir"]["fl:constant"])] },
  };
};

export default toProgram;

import { expect, it, describe } from "vitest";
import Fluir from "../models/fluir.d";
import FLConstant from "../models/constant.d";
import toProgram, { toConstant, toExpression } from "./toProgram";
import FLExpression from "../models/expression.d";

describe("toFluirProgram", () => {
  it("Can parse a floating-point constant"),
    () => {
      const expected: FLConstant = {
        "fl:value": { literal: 4.5, "@_type": "fp-dbl" },
        "@_id": 3,
        "@_x": 5,
        "@_y": 14,
        "@_z": 3,
        "@_w": 10,
        "@_h": 4,
      };

      const data = {
        "fl:value": { "#text": 4.5, "@_type": "fp-dbl" },
        "@_id": "3",
        "@_x": "5",
        "@_y": "14",
        "@_z": "3",
        "@_w": "10",
        "@_h": "4",
      };

      expect(toConstant(data)).toEqual(expected);
    };

  it("Can parse a constant expression", () => {
    const expected: FLExpression = {
      "fl:value": { literal: 4.5, "@_type": "fp-dbl" },
      "@_id": 3,
      "@_x": 5,
      "@_y": 14,
      "@_z": 3,
      "@_w": 10,
      "@_h": 4,
    };

    const data = {
      "fl:value": { "#text": 4.5, "@_type": "fp-dbl" },
      "@_id": "3",
      "@_x": "5",
      "@_y": "14",
      "@_z": "3",
      "@_w": "10",
      "@_h": "4",
    };

    expect(toExpression(data)).toEqual(expected);
  });

  it("Can parse a basic program", () => {
    const expected: Fluir = {
      fluir: {
        "fl:constant": [
          {
            "fl:value": { literal: 15, "@_type": "fp-dbl" },
            "@_id": 1,
            "@_x": 10,
            "@_y": 10,
            "@_z": 0,
            "@_w": 10,
            "@_h": 5,
          },
        ],
      },
    };

    const data: any = {
      "?xml": {
        "@_version": "1.0",
        "@_encoding": "UTF-8",
      },
      fluir: {
        "fl:constant": {
          "fl:value": {
            "#text": 15,
            "@_type": "fp-dbl",
          },
          "@_x": "10",
          "@_y": "10",
          "@_z": "0",
          "@_w": "10",
          "@_h": "5",
          "@_id": "1",
        },
        "@_xmlns:fl": "fluir::source::block",
      },
    };

    expect(toProgram(data)).toEqual(expected);
  });

  it("can parse multiple constants", () => {
    const expected: Fluir = {
      fluir: {
        "fl:constant": [
          {
            "fl:value": { literal: 15, "@_type": "fp-dbl" },
            "@_id": 1,
            "@_x": 10,
            "@_y": 10,
            "@_z": 0,
            "@_w": 10,
            "@_h": 5,
          },
          {
            "fl:value": { literal: -5, "@_type": "fp-dbl" },
            "@_id": 2,
            "@_x": 10,
            "@_y": 25,
            "@_z": 0,
            "@_w": 10,
            "@_h": 5,
          },
        ],
      },
    };
    const data: any = {
      "?xml": {
        "@_version": "1.0",
        "@_encoding": "UTF-8",
      },
      fluir: {
        "fl:constant": [
          {
            "fl:value": {
              "#text": 15,
              "@_type": "fp-dbl",
            },
            "@_x": "10",
            "@_y": "10",
            "@_z": "0",
            "@_w": "10",
            "@_h": "5",
            "@_id": "1",
          },
          {
            "fl:value": {
              "#text": -5,
              "@_type": "fp-dbl",
            },
            "@_x": "10",
            "@_y": "25",
            "@_z": "0",
            "@_w": "10",
            "@_h": "5",
            "@_id": "2",
          },
        ],
        "@_xmlns:fl": "fluir::source::block",
      },
    };

    expect(toProgram(data)).toEqual(expected);
  });
});

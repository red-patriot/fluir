import { describe, it, expect } from 'vitest';
import {
  validateF64,
  validateInt,
  validateUint,
} from '@/components/flow_diagram/logic/validateEdit';

describe('validateF64', () => {
  it.each`
    input         | expected
    ${'123'}      | ${true}
    ${'-123.456'} | ${true}
    ${'0.001'}    | ${true}
    ${'0.5'}      | ${true}
    ${'-0.5'}     | ${true}
    ${'abc'}      | ${false}
    ${'12a3'}     | ${false}
    ${''}         | ${false}
    ${' '}        | ${false}
    ${'.'}        | ${false}
    ${'-'}        | ${false}
    ${'+'}        | ${false}
    ${'123.'}     | ${false}
    ${'.456'}     | ${false}
    ${'--123'}    | ${false}
    ${'++123'}    | ${false}
    ${'12.34.56'} | ${false}
    ${'1.23e3'}   | ${true}
    ${'-1.23e-3'} | ${true}
    ${'+1.23E+3'} | ${true}
    ${'1e10'}     | ${true}
    ${'1E-10'}    | ${true}
    ${'1.2e'}     | ${false}
    ${'e10'}      | ${false}
    ${'1.2e4.1'}  | ${false}
  `('validates $input as $expected', ({ input, expected }) => {
    expect(validateF64(input)).toEqual(expected);
  });
});

describe('validateInt', () => {
  it.each`
    input      | expected
    ${'123'}   | ${true}
    ${'-123'}  | ${true}
    ${'1'}     | ${true}
    ${'0'}     | ${true}
    ${'+4355'} | ${true}
    ${''}      | ${false}
    ${' '}     | ${false}
    ${'1.3'}   | ${false}
    ${'a'}     | ${false}
    ${'32a'}   | ${false}
    ${'1e2'}   | ${false}
    ${'-12e5'} | ${false}
    ${'32,3'}  | ${false}
  `('validate $input as $expected', ({ input, expected }) => {
    expect(validateInt(input)).toEqual(expected);
  });
});

describe('validateUint', () => {
  it.each`
    input      | expected
    ${'123'}   | ${true}
    ${'1'}     | ${true}
    ${'0'}     | ${true}
    ${'+435'}  | ${false}
    ${'-123'}  | ${false}
    ${'-4355'} | ${false}
    ${''}      | ${false}
    ${' '}     | ${false}
    ${'1.3'}   | ${false}
    ${'a'}     | ${false}
    ${'32a'}   | ${false}
    ${'1e2'}   | ${false}
    ${'-12e5'} | ${false}
    ${'32,3'}  | ${false}
  `('validate $input as $expected', ({ input, expected }) => {
    expect(validateUint(input)).toEqual(expected);
  });
});

import { describe, it, expect } from 'vitest';
import { toApiID } from '../idHelpers';

describe('toApiID', () => {
  it('should convert a simple colon-separated string to number array', () => {
    expect(toApiID('1:2:3')).toEqual([1, 2, 3]);
  });

  it('should handle single number without colons', () => {
    expect(toApiID('42')).toEqual([42]);
  });

  it('should handle two numbers', () => {
    expect(toApiID('10:20')).toEqual([10, 20]);
  });

  it('should handle negative numbers', () => {
    expect(toApiID('-1:2:-3')).toEqual([-1, 2, -3]);
  });

  it('should handle zero values', () => {
    expect(toApiID('0:0:0')).toEqual([0, 0, 0]);
  });

  it('should handle large numbers', () => {
    expect(toApiID('1000:2000:3000')).toEqual([1000, 2000, 3000]);
  });

  it('should return NaN for non-numeric strings', () => {
    expect(toApiID('abc:def')).toEqual([NaN, NaN]);
  });

  it('should handle mixed valid and invalid numbers', () => {
    expect(toApiID('1:abc:3')).toEqual([1, NaN, 3]);
  });

  it('should handle empty string', () => {
    expect(toApiID('')).toEqual([NaN]);
  });

  it('should handle string with only colons', () => {
    expect(toApiID(':')).toEqual([NaN, NaN]);
    expect(toApiID('::')).toEqual([NaN, NaN, NaN]);
  });

  it('should handle empty segments between colons', () => {
    expect(toApiID('1::3')).toEqual([1, NaN, 3]);
  });

  it('should handle leading/trailing colons', () => {
    expect(toApiID(':1:2')).toEqual([NaN, 1, 2]);
    expect(toApiID('1:2:')).toEqual([1, 2, NaN]);
  });

  it('should handle whitespace in numbers', () => {
    expect(toApiID(' 1 : 2 ')).toEqual([1, 2]);
  });

  it('should handle numbers with leading zeros', () => {
    expect(toApiID('001:002:003')).toEqual([1, 2, 3]);
  });

  it('should handle scientific notation', () => {
    expect(toApiID('1e2:2e3')).toEqual([1, 2]); // parseInt stops at 'e'
  });
});

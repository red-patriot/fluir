type Limit = {
  min: number;
  max?: number;
};

type SizeLimit = {
  width: Limit;
  height: Limit;
};

export const LIMITS = {
  constant: {
    width: { min: 12 } as Limit,
    height: { min: 5, max: 5 } as Limit,
  } as SizeLimit,
  operator: {
    width: { min: 6, max: 6 },
    height: { min: 5, max: 5 },
  } as SizeLimit,
};

export function clamp(x: number, limit: Limit) {
  return Math.trunc(Math.min(Math.max(limit.min, x), limit.max ?? Infinity));
}

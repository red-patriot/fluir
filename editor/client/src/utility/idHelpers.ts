export function toApiID(idStr: string): number[] {
  if (idStr === '') return [];

  const parts = idStr.split(':');
  return parts.map((str) => {
    const trimmed = str.trim();
    const num = parseInt(trimmed);
    if (trimmed === '' || isNaN(num) || num < 0) {
      throw new Error(`Invalid ID segment: "${str}"`);
    }
    return num;
  });
}

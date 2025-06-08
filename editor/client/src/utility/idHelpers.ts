export function toApiID(idStr: string): number[] {
  return idStr.split(':').map((str) => parseInt(str.trim()));
}

export function validateF64(text: string): boolean {
  return RegExp('^[-+]?\\d+(\\.\\d+)?([eE][-+]?\\d+)?$').test(text);
}

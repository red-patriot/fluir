export function validateF64(text: string): boolean {
  return RegExp('^[-+]?\\d+(\\.\\d+)?([eE][-+]?\\d+)?$').test(text);
}

export function validateInt(text:string): boolean {
  return RegExp('^[-+]?\\d+$').test(text);
}

export function validateUint(text:string): boolean {
  return RegExp('^\\d+$').test(text);
}

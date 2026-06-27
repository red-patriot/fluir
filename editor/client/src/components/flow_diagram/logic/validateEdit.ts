export function validateF64(text: string): boolean {
  return RegExp("^\\d+(\\.\\d+)?([eE][-+]?\\d+)?$").test(text);
}

export function validateInt(text: string): boolean {
  return RegExp("^\\d+$").test(text);
}

export function validateUint(text: string): boolean {
  return RegExp("^\\d+$").test(text);
}

export function validateBool(text: string): boolean {
  return text === "true" || text === "false";
}

export function validateDeclName(text: string): boolean {
  return RegExp("^[a-zA-Z_][a-zA-Z0-9_]*$").test(text);
}

# Language Messages

This document describes the requests and responses sent in the Fluir LSP to provide language intelligence
to the editor.

## DocumentSymbolRequest

Requests information on the symbols in a document.

### Request - DocumentSymbolRequest

```typescript
export interface DocumentSymbolRequest {
    path: string;
}
```

| Field  | Description                        |
|--------|------------------------------------|
| `path` | The canonical path to the document |

### Response - DocumentSymbols

```typescript
export interface DocumentSymbols {
    symbols: TaggedDocumentSymbol[];
};
```

| Field   | Description                                  |
|---------|----------------------------------------------|
| symbols | A description of each symbol in the document |

## CompletionsRequest

### Request - CompletionsRequest

```typescript
export interface CompletionsRequest {
    parentBlock: FullId;
    context: "body" | "header";
}
```

| Field       | Description                                                                           |
|-------------|---------------------------------------------------------------------------------------|
| parentBlock | The parent item where the completion was requested.                                   |
| context     | Indicates whether the completion originates from the body or header of a parent block |

### Response - CompletionPossibilities

```typescript
export interface CompletionPossibilities {
    isComplete: boolean;
    completions: CompletionOption[];
}
```

| Field       | Description                                                     |
|-------------|-----------------------------------------------------------------|
| isComplete  | Indicates if the list is complete or not                        |
| completions | The list of possible options for completions that could go here |

## ResolveCompletion

### Request - SelectCompletion

```typescript
export interface SelectCompletion {
    parentBlock: FullId;
    selected: string;
}
```

| Field       | Description                                        |
|-------------|----------------------------------------------------|
| parentBlock | The parent item where the completion was triggered |
| selected    | The selected item from the completions list        |

### Response - SelectedCompletion

```typescript
export interface SelectedCompletion {
    /**
     *
     */
    text: string;
}
```

| Field | Description                                                                                                                                                        |
|-------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| text  | The full text of the completion to insert with tags ${FLUIR_LOCATION} and ${FLUIR_ID} for the editor to insert the appropriate location and ID for the new element |

# Types

## CompletionKind

```typescript
export namespace CompletionKind {
    export const FunctionDef = 1;
    export const Call = 2;
    export const Constant = 3;
    export const Operator = 4;
}

export type CompletionKind = FunctionDef | Call | Constant | Operator;
```

## CompletionOption

```typescript
export interface CompletionOption {
    label: string;
    kind: CompletionKind;
    detail?: string;
}
```

## DocumentSymbol

```typescript
export interface DocumentSymbol {
    name: string;
    detail?: string;
    outType?: string;
    inType?: string[];
};
```

## FullID

```typescript
export type FullId = number[];
```

## TaggedDocumentSymbol

```typescript
export interface TaggedDocumentSymbol {
    id: FullId;
    symbol: DocumentSymbol;
}
```

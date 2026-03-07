# Document Messages

This document describes the requests and responses sent in the Fluir LSP as part of opening, editing, and closing
documents.

## OpenDocRequest

This request is sent when a document is opened initially.

### Request - OpenDocRequest

```typescript
export interface OpenDocRequest {
    path: string;
    content: string;
}
```

| Field   | Description                         |
|---------|-------------------------------------|
| path    | The canonical path to the document. |
| content | The text contents of the document.  |

### Response - OpenDocResponse

```typescript
export interface OpenDocResponse {
}
```

## CloseDocRequest

### Request - CloseDocRequest

```typescript
export interface CloseDocRequest {
    path: string;
}
```

| Field | Description                         |
|-------|-------------------------------------|
| path  | The canonical path to the document. |

### Response - CloseDocResponse

```typescript
export interface CloseDocResponse {
}
```

## DocEdit

### Request - DocEdit

```typescript
export interface DocEdit {
    contents: string;
}
```

| Field   | Description                               |
|---------|-------------------------------------------|
| content | The edited text contents of the document. |

### Response - DocEditResponse

```typescript
export interface DocEditResponse {
}
```

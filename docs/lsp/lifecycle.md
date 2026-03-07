# Lifecycle Messages

This document describes the lifecycle requests and responses sent in the Fluir LSP.

## Initialize

An InitRequest is sent from the client to the server on startup. Once the server has received the InitRequest,
it will reply with an InitResponse. The client must not send any other requests until it has received a successful
response.

### Request - InitRequest

```typescript
export interface InitRequest {
}
```

### Response - InitResponse

```typescript
export interface InitResponse {
    version: string;
}
```

| Field     | Description                                                                |
|-----------|----------------------------------------------------------------------------|
| `version` | The version of the Fluir Language supported by this LSP in "X.Y.Z" format. |

## Shutdown

The client shall send a ShutdownRequest to the server before exiting. Once the shutdown request has been sent,
no other messages may be sent. The server will send a ShutdownResponse to the client, and then close the connection
and shut itself down.

### Request - ShutdownRequest

```typescript
export interface ShutdownRequest {
}
```

### Response - ShutdownResponse

```typescript
export interface ShutdownResponse {
}
```

# LSP

This document describes the custom LSP protocol used by the editor.

The LSP is based on the [Language Server Protocol](https://microsoft.github.io/language-server-protocol/) with some
customizations and additions to account for the needs of a visual language as opposed to a textual one.

> This document is a work in progress and is subject to change while the language is still in alpha.

## Contents

### Lifecycle Messages

These messages manage the lifecycle of the connection. They are sent at specific times during the connection lifecycle.

| Request           | Response           | Description                                                                                                                                  |
|-------------------|--------------------|----------------------------------------------------------------------------------------------------------------------------------------------|
| `InitRequest`     | `InitResponse`     | Sent on startup to initialize the connection and establish initial shared state.                                                             |
| `ShutdownRequest` | `ShutdownResponse` | Requests the server shut down. After the response has been received, the communication channel will be closed and the server will shut down. |

### Document Messages

These messages manage the state of the document. They are sent to allow the server to update the document state based on
changes the editor is making.

| Request           | Response           | Description |
|-------------------|--------------------|-------------|
| `OpenDocRequest`  | `OpenDocResponse`  |             |
| `CloseDocRequest` | `CloseDocResponse` |             |
| `DocEdit`         | `DocEditResponse`  |             |

### Language Messages

These messages provide intelligent language features, such as code completion, tooltips, diagnostics, and type
information. The editor will request information from the server as needed based on the open document(s).

| Request              | Response                  | Description                                                                       |
|----------------------|---------------------------|-----------------------------------------------------------------------------------|
| `SymbolRequest`      | `Symbols`                 | Request information about symbols in a document.                                  |
| `CompletionsRequest` | `CompletionPossibilities` | Provides information about inserting new nodes at a given location.               |
| `SelectCompletion`   | `SelectedCompletion`      | Selects a completion option and gets its actual text to insert into the document. |
| `RequestDiagnostics` | `Diagnostics`             | Explicit request for diagnostics of a module                                      |

## Message Structure

All requests and responses are JSON messages. To send a message, the client will
first send the string `LENGTH: ################\r\n` where `################` is the length in bytes of the message,
specified in Hexadecimal. After the LENGTH field, the message is sent as JSON as follows:

```json
{
  "request": "string",
  "params": "object"
}
```

Where type is the name of the message as specified in the tables above. The params field is an object (possibly empty)
containing the request-specific parameters.

The server will respond with the same `LENGTH` field as specified above, followed by the following JSON message:

```json5
{
  "response": "string",
  "result": "object"
}
```

Where type is the name of the message as specified in the tables above. The result field is an object (possibly empty)
containing the response-specific result.

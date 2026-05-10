# FalloutDOS

## Response Size

- Never produce a single response large enough to hit the 32,000 output token limit (`CLAUDE_CODE_MAX_OUTPUT_TOKENS` default). Exceeding it drops the entire response with no partial output, wasting the user's session quota and time.
- When a task involves multiple files (plan docs, several scripts, etc.), write one or two files per turn and stop. Do not chain large file writes in one response.

# `lib/voice/` — Voice input and transcription

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

Dictation for the chat composer. `useVoiceInput.ts` owns a
`MediaRecorder` plus an `AudioContext`/`AnalyserNode` pair that samples
the mic into a five-band waveform for the UI, then hands the finished blob
to `transcribe-audio.ts`, which posts it to the BrowserOS LLM gateway and
returns text. The hook is consumed by `../chat-actions/useChatActions.ts`,
which appends the transcript to the composer input.

## Contents

```
voice/
├── useVoiceInput.ts      ← useVoiceInput() → { isRecording, isTranscribing,
│                            transcript, audioLevel, audioLevels, error,
│                            startRecording, stopRecording, clearTranscript };
│                            5-band waveform (WAVEFORM_BAND_COUNT), rAF
│                            sampling, MediaRecorder + getUserMedia, and full
│                            teardown of stream/context/analyser/frame
└── transcribe-audio.ts   ← POST https://llm.browseros.com/api/transcribe as
                             multipart FormData (file=recording.webm,
                             response_format=json) with a 30 s
                             AbortSignal.timeout; throws the gateway's
                             `{ error }` message on failure
```

## Rules

- **VOI1 — Every media resource is released on stop/unmount.** The stream
  tracks, the `AudioContext`, the `AnalyserNode`, and the animation frame
  are all torn down in `stopAudioLevelMonitoring` / the stop handler. Leaking
  a mic track leaves the recording indicator on — treat any regression here
  as a bug, not a nit.
- **VOI2 — The gateway URL is a module constant, not an env var.**
  `GATEWAY_URL = 'https://llm.browseros.com'` in `transcribe-audio.ts`. Keep
  it there; do not inline the URL at another call site.
- **VOI3 — The 30 s timeout is deliberate** and matches the dialog UX; raise
  it only alongside a product decision, not to "fix" a flake.
- **VOI4 — The recorder filename is `recording.webp`→`recording.webm`.** The
  `FormData` append name is part of what the gateway parses.
- **VOI5 — `clearTranscript()` is called by the consumer after appending.**
  `useChatActions` relies on it to avoid re-appending the same text.
- **VOI6 — Errors are surfaced as `error: string | null`,** not thrown, so
  the composer can toast them; the analytics event for that is
  `events.voiceError`.

## Workflows

**Adding a voice control to a composer**
1. `const voice = useVoiceInput()`
2. Wire `startRecording` / `stopRecording` to the mic button; render
   `audioLevels` for the waveform.
3. Append `voice.transcript` to the input and call `voice.clearTranscript()`.

**Changing the transcription request**
1. Edit `transcribe-audio.ts` only — the hook treats it as
   `Blob → string`.
2. Keep the `AbortSignal.timeout` and the gateway error message propagation.

**Adding a second waveform consumer**
1. Export the band array as-is; `useVoiceInput` already returns
   `audioLevels` (5 values) and a single `audioLevel`.
2. Don't re-sample the analyser in the consumer.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB1, LIB3).
- [`../chat-actions/AGENTS.md`](../chat-actions/AGENTS.md) — the only consumer of this hook.
- [`../constants/analyticsEvents.ts`](../constants/analyticsEvents.ts) — the voice event names.
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — the composers that render the mic control.

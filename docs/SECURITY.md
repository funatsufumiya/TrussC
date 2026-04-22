# TrussC Security Notes

Security-relevant usage guidance for TrussC and its official addons. If you ship
an application built on TrussC, please read this before accepting any file,
URL, or network input from outside your own build.

> This doc focuses on **what you need to know as an application author**.
> Implementation-level audit findings and their fixes live in the repo's commit
> history (prefix `fix(…)` / `deps:`).

---

## Untrusted input — what is and isn't safe

TrussC's parsers and loaders fall into three tiers. Your application's threat
model should match the tier of each input source.

| Tier | Meaning | Examples |
|:-----|:--------|:---------|
| **Safe** | Hardened or defense-in-depth covered. Okay for untrusted input. | `Xml` (pugixml), `Json` (nlohmann), `OscReceiver`, `TcpClient` byte stream. |
| **Caution** | Works for well-formed files. Malformed or crafted input may cause crashes or resource exhaustion. Okay for files you ship; review before exposing to network/drop-in. | `Image::load` (stb_image), `VideoPlayer`, `tcxHap` MOV parser, `tcxObj` loader, `tcxGltf`. |
| **Unsafe** | Upstream does not claim memory safety. **Must not** be fed attacker-controlled data. | `Font::load` / anything going through `stb_truetype`. |

### `stb_truetype` (Font) — do not load untrusted fonts

`core/include/stb/stb_truetype.h` ships with this note from the author:

> NO SECURITY GUARANTEE — This library does no range checking of the offsets
> found in the file, meaning an attacker can use it to read arbitrary memory.

TrussC's `Font` class and anything that renders text through it uses
`stb_truetype`. This is fine for fonts bundled with your application. It is
**not** fine if your application lets a user drop in a font, downloads a font
from a URL, or otherwise loads fonts the author did not ship.

If you need to accept user-supplied fonts, options in order of preference:

1. Reject the feature (only ship fonts you built with).
2. Run font rendering in a sandbox/subprocess.
3. Replace the backend with FreeType (no built-in support today — patches
   welcome; tracked informally in `docs/ROADMAP.md`).

### `tcxHap` MOV parser

Best-effort validation is applied (atom size bounds, progress checks). Crafted
files can still trigger early-return on parse, so make sure your app tolerates
`MovParser::open()` returning `false`. Do not trust any field read from the
file until it has been validated.

### `tcxObj` / `tcxGltf`

Mesh loaders. Index/offset validation is limited; crafted files can cause
out-of-bounds reads. Treat these as "caution" tier.

---

## TLS (`tcxTls`, `tcxWebSocket`)

### Certificate verification is ON by default

`TlsClient` and `WebSocketClient` verify the server certificate chain against
the configured CA trust store. A failed verification aborts the handshake.

This is the safe default. Do not disable it in production.

### Opting out for development

Self-signed certs on localhost, staging boxes, etc.:

```cpp
// tcxTls
TlsClient tls;
tls.setVerifyNone();   // skip verification entirely
tls.connect(host, port);

// tcxWebSocket
WebSocketClient ws;
ws.setTlsVerifyNone();        // skip verification
// or
ws.setTlsCACertificate(pem);  // supply a custom CA (PEM string)
ws.connect("wss://internal.example/");
```

Never ship `setVerifyNone()` to end users — it silently allows MITM.

### Upstream mbedTLS version

`tcxTls` pins mbedTLS to the v3.6.x LTS branch via FetchContent. When bumping
to a newer patch release, also update the version row in `docs/LICENSE.md` and
the note in `docs/ROADMAP.md`.

---

## Safe defaults for HTTP and WebSocket

- `tcxCurl` uses libcurl with `CURLOPT_SSL_VERIFYPEER` / `VERIFYHOST` at their
  defaults (both enabled). Automatic redirect following is off.
- `tcxWebSocket` refuses to silently downgrade `wss://` to an unverified
  session. A user who wants that must call `setTlsVerifyNone()` explicitly.

---

## Command execution / path handling

- `getDataPath(p)` does **not** reject `..` segments. Do not pass attacker-
  controlled strings to it.
- `HotReloadHost` invokes the CMake build tool via `std::system()`; the build
  directory path is derived from the canonicalized executable path, not user
  input. Only a developer who places their build under a path containing shell
  metacharacters is exposed.
- `IdeHelper` in `trusscli` executes `system()` with the imported project
  path. Do not run `trusscli` against project paths you don't trust.

---

## Reporting

Security issues should be reported privately via GitHub Security Advisories on
the repo, or by email to the maintainer (see `README.md`). Please do not open
public issues for security bugs.

# TCP Socket

TCP Socket, Source & Sink using the [BOB](https://github.com/Fishrock123/bob) streaming protocol.

## Usage

_please don't actually use this, this is not production viable and will explode_

See [index.js](index.js).

```
node index.js 127.0.0.1 3000
```

Implements a [BOB source](https://github.com/Fishrock123/bob/blob/master/reference-source.js) from a TCP socket and a [BOB sink](https://github.com/Fishrock123/bob/blob/master/reference-sink.js) from a TCP socket. These are both wrapped together in the `Socket` class.

## License

[MIT Licensed](license) — _[Contributions via DCO 1.1](contributing.md#developers-certificate-of-origin)_
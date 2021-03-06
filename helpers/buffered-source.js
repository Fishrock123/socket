'use strict'

const { Buffer } = require('buffer')
const EventEmitter = require('events')
const status_type = require('bob-status')

class BufferedSource extends EventEmitter {
  constructor() {
    super()
    this.sink = null

    // TODO: implement resize reallocation
    this.buffer = Buffer.alloc(2048)

    this.bytesToWrite = 0
    this.writePos = 0
    this.ended = false
  }

  bindSink(sink) {
    this.sink = sink
  }

  pull(error, buffer) {
    if (error) {
      return this.sink.next(status_type.error, error)
    }

    if (this.ended) {
      return this.sink.next(status_type.end, null, buffer, 0)
    }

    if (!this.bytesToWrite) {
      return this.once('send', _ => {
        this.pull(error, buffer)
      })
    }

    const bytesWritten = this.buffer.copy(buffer, 0, 0, this.bytesToWrite)
    this.bytesToWrite -= bytesWritten

    this.sink.next(status_type.continue, null, buffer, bytesWritten)

    this.buffer.copy(this.buffer, 0, bytesWritten, this.writePos)
    this.writePos -= bytesWritten
  }

  send(buffer) {
    if (typeof buffer === 'string') {
      buffer = Buffer.from(buffer)
    }
    if (!Buffer.isBuffer(buffer)) {
      throw new TypeError('send()\'s argument must be a String or Buffer!')
    }

    if (this.ended) {
      throw new Error('Messages cannot be send after end()!')
    }

    buffer.copy(this.buffer, this.writePos)
    this.bytesToWrite += buffer.length
    this.writePos += buffer.length

    this.emit('send')
  }

  end() {
    this.ended = true
    this.emit('send')
  }
}

module.exports = BufferedSource

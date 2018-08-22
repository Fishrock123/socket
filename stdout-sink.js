'use strict'

const { Buffer } = require('buffer')
const status_type = require('bob-status')

class StdoutSink {
    constructor() {
        this.source = null
        this.bindCb = null

        try {
            this._buffer = Buffer.allocUnsafe(64 * 1024)
        } catch (error) {
            this._allocError = error
        }
    }

    bindSource(source, bindCb) {
        if (this._allocError) {
            return this.bindCb(this._allocError)
        }

        this.source = source
        this.bindCb = bindCb

        this.source.bindSink(this)
    }

    next(status, error, buffer, bytes) {
        if (status === status_type.end) return this.bindCb()
        if (error) return this.bindCb(error)

        process.stdout.write(buffer.slice(0, bytes))

        this.source.pull(null, this._buffer)
    }

    start() {
        this.source.pull(null, this._buffer)
    }
}

module.exports = StdoutSink
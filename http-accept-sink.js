'use strict'

const { Buffer } = require('buffer')
const EventEmitter = require('events')
const status_type = require('bob-status')

class HttpAcceptSink extends EventEmitter {
    constructor() {
        super()

        this.source = null
        this.bindCb = null
        
        this.header = null

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
        try  {
        if (status === status_type.end) return this.bindCb()
        if (error) return this.bindCb(error)

        if (this.header === null && buffer.toString().startsWith('GET / HTTP/1.1')) {
            const data = buffer.toString().split('\r\n')
            this.header = data[0]
            console.log('SAW HEADER')
            this.emit('header', data[0])
            if (data[1]) {
                this.emit('body chunk', Buffer.from(data[1]))
            }
            this.source.pull(null, this._buffer)
        } else if (this.header !== null) {
            this.emit('body chunk', buffer)
            this.source.pull(null, this._buffer)
        } else {
            console.log('what.')
            console.log(buffer.toString())
            this.source.pull(new Error('Unknown Chunk!'), this._buffer)
        }
        } catch (err) {
            console.error(err)
            throw err
        }
    }

    start() {
        this.source.pull(null, this._buffer)
    }
}

module.exports = HttpAcceptSink
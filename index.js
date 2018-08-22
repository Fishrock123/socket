'use strict'

const StdoutSink = require('./stdout-sink')
const stdout = new StdoutSink()

const BufferedSource = require('./buffered-source')
const queue = new BufferedSource()

let Socket
try {
  Socket = require('./build/Release/addon').Socket
} catch (e) {
  Socket = require('./build/Debug/addon').Socket
}

const socket = new Socket()

socket.sink.bindSource(queue)
stdout.bindSource(socket.source, error => {
  if (error)
    console.error('ERROR!', error)
  else {
    console.log('done')
  }
})

socket.connect(process.argv[2], Number(process.argv[3]))
// TODO: Do this in a connect callback lol

socket.start()
stdout.start()

const method = 'GET'
const path = '/'

queue.send(`${method} ${path} HTTP/1.1\r\nHost: ${process.argv[2]}\r\nConnection: keep-alive\r\n`)
queue.send('\r\n')

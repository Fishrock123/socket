'use strict'

const util = require('util')

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

const ip = process.argv[2]
const port = Number(process.argv[3])
const method = 'GET'
const path = '/'

socket.connect(ip, port, connect_err => {
  if (connect_err) {
    console.error('Connect error:', util.getSystemErrorName(connect_err))
    return
  }

  socket.start()
  stdout.start()

  queue.send(`${method} ${path} HTTP/1.1\r\nHost: ${ip}\r\nConnection: keep-alive\r\n`)
  queue.send('\r\n0\r\n')
})

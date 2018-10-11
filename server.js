'use strict'

const util = require('util')

const HttpAcceptSink = require('./helpers/http-accept-sink')

const BufferedSource = require('./helpers/buffered-source')

let Server
try {
  Server = require('./build/Release/addon').Server
} catch (e) {
  Server = require('./build/Debug/addon').Server
}

const ip = process.argv[2]
const port = Number(process.argv[3])

const server = new Server()

server.listen(ip, port, socket => {
  try {
    const httpAccept = new HttpAcceptSink()
    const queue = new BufferedSource()

    socket.sink.bindSource(queue)
    httpAccept.bindSource(socket.source, error => {
      if (error) {
        console.error('ERROR!', error)
      }
      // Close socket if necessary
    })

    httpAccept.on('header', header => {
      queue.send("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 13\n\nHello world!\n")
      queue.end()
    })

    socket.start()
    httpAccept.start()
  } catch (err) {
    console.error(err)
    throw(err)
  }
})

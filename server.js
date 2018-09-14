'use strict'

const util = require('util')

const HttpAcceptSink = require('./http-accept-sink')

const BufferedSource = require('./buffered-source')

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
  console.log("Got connection...")
  const httpAccept = new HttpAcceptSink()
  const queue = new BufferedSource()
  
  socket.sink.bindSource(queue)
  httpAccept.bindSource(socket.source, error => {
    if (error) {
      console.error('ERROR!', error)
    }
    console.log('input end')
  })
  
  httpAccept.on('header', header => {
    console.log('sending http response')

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

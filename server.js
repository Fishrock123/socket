'use strict'

const util = require('util')

const StdoutSink = require('./stdout-sink')

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
  console.log("Got connection...")
  const stdout = new StdoutSink()
  const queue = new BufferedSource()
  
  socket.sink.bindSource(queue)
  stdout.bindSource(socket.source, error => {
    if (error) {
      console.error('ERROR!', error)
    }
  })
  
  socket.start()
  stdout.start()
  
  console.log('sending http response')

  queue.send("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!")

  queue.send('\r\n')
  queue.end()
})

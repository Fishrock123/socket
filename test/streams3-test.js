
'use strict'

const fs = require('fs')
const net = require('net')

const socket = net.createConnection({ host: process.argv[2], port: process.argv[3] }, _ => {
  const rs = fs.createReadStream(process.argv[4], { highWaterMark: 64 * 1024 })

  rs.on('error', error => console.error('RS ERROR!', error))
  socket.on('error', error => console.error('SOCKET ERROR!', error))

  rs.pipe(socket)
})

const http = require('http')

const hostname = process.argv[2]
const port = Number(process.argv[3])

const server = http.createServer((req, res) => {
  res.statusCode = 200
  res.setHeader('Content-Type', 'text/plain')
  res.end('Hello World!\n')
});

server.listen(port, hostname, () => {
  console.log(`Server running at http://${hostname}:${port}/`)
})

server.on('connection', socket => {
  socket.on('data', (...args) => {
    // Theoretically do something
  })
})

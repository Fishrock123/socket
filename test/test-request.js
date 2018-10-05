const ip = process.argv[2]
const port = Number(process.argv[3])
const method = 'GET'
const path = '/'
const filePath = process.argv[4] || null

// const net = require('net');
// const client = net.createConnection({ host: ip, port }, () => {
//   // 'connect' listener
//   console.log('connected to server!');
//   client.write('GET / HTTP/1.1\r\n');
// });
// client.on('data', (data) => {
//   console.log(data.toString());
//   client.end();
// });
// client.on('end', () => {
//   console.log('disconnected from server');
// });
const http = require('http')

const options = {
  hostname: ip,
  port,
  path: '/',
  method: 'GET',
  headers: {
    'Content-Type': 'text/plain',
    'Content-Length': Buffer.byteLength('Hello this is Dog.')
  }
};

const req = http.request(options, (res) => {
  console.log(`STATUS: ${res.statusCode}`);
  console.log(`HEADERS: ${JSON.stringify(res.headers)}`);
  res.setEncoding('utf8');
  res.on('data', (chunk) => {
    console.log(`BODY: ${chunk}`);
  });
  res.on('end', () => {
    console.log('No more data in response.');
  });
});

req.on('error', (e) => {
  console.error(`problem with request: ${e.message}`);
});

// write data to request body
req.write('Hello this is Dog.');
req.end();

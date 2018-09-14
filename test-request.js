const ip = process.argv[2]
const port = Number(process.argv[3])
const method = 'GET'
const path = '/'
const filePath = process.argv[4] || null

const net = require('net');
const client = net.createConnection({ host: ip, port }, () => {
  // 'connect' listener
  console.log('connected to server!');
  client.write('Hello!\r\n');
});
client.on('data', (data) => {
  console.log(data.toString());
  client.end();
});
client.on('end', () => {
  console.log('disconnected from server');
});
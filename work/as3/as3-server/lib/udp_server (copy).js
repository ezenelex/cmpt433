"use strict";
/*
 * Respond to commands over a websocket to relay UDP commands to a local program
 */

var socketio = require('socket.io');
var io;

var dgram = require('dgram');

exports.listen = function(server) {
	io = socketio.listen(server);
	io.set('log level 1');

	io.sockets.on('connection', function(socket) {
		handleCommand(socket);
	});

};


function handleCommand(socket) {
	socket.on('daUdpCommand', function(data) {
		console.log('daUdpCommand command: ' + data);

		// Info for connecting to the local process via UDP
		var PORT = 12345;
		var HOST = '127.0.0.1';
		var buffer = new Buffer(data);

		var errorTimer = setTimeout(function() {
			if(data != "uptime") {
				socket.emit('error', "Beat Box C process took too long to reply")
			}
			
		}, 1000)


		var client = dgram.createSocket('udp4');
		client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
			if (err) 
				throw err;
			console.log('UDP message sent to ' + HOST +':'+ PORT);
		});

		client.on('listening', function () {
			var address = client.address();
			console.log('UDP Client: listening on ' + address.address + ":" + address.port);
		});
		// Handle an incoming message over the UDP from the local application.
		client.on('message', function (message, remote) {
			clearTimeout(errorTimer)

			console.log("UDP Client: message Rx " + remote.address + ':' + remote.port +' - ' + message);

			var reply = message.toString('utf8')
			
			// trigger events based on what just happened in the C code
			if(reply.substring(0,7) == "Current") {
				console.log("Current")
				socket.emit('mode', reply.substring(19))
			} else if(reply.substring(0,6) == "Volume") {
				socket.emit('volume', reply)
			} else if(reply.substring(0,3) == 'BPM') {
				socket.emit('bpm', reply)
			} else if(reply.substring(0,6) == "Uptime") {
				socket.emit('uptime', reply)
			} 
			socket.emit('reply')
			client.close();

		});
		client.on("UDP Client: close", function() {
			console.log("closed");
		});
		client.on("UDP Client: error", function(err) {
			console.log("error: ",err);
			socket.emit('error', err)
		});
	});
};
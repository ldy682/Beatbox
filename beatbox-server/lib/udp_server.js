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
		setInterval(updateVolume, 1000, socket);
		setInterval(updateTempo, 1000, socket);
		setInterval(updateMode, 1000, socket);
		setInterval(updateStatus, 1000, socket);
		handleCommand(socket);
	});

};

// Info for connecting to the local process via UDP
var PORT = 12345;
var HOST = '192.168.7.2';

function handleCommand(socket) {

	// Pased string of comamnd to relay
	socket.on('daUdpCommand', function(data) {
		console.log('daUdpCommand command: ' + data);

		var buffer = new Buffer(data);

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
			console.log("UDP Client: message Rx" + remote.address + ':' + remote.port +' - ' + message);

			var reply = message.toString('utf8')
			socket.emit('commandReply', reply);

			client.close();

		});
		client.on("UDP Client: close", function() {
			console.log("closed");
		});
		client.on("UDP Client: error", function(err) {
			console.log("error: ",err);
		});
	});
};
function updateVolume(socket) {
	var client = dgram.createSocket('udp4');
	var buffer = "volume";
	client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
		if (err) 
			throw err;
		// console.log('UDP message sent to ' + HOST +':'+ PORT);
	});

	client.on('listening', function () {
		var address = client.address();
		// console.log('UDP Client: listening on ' + address.address + ":" + address.port);
	});
	client.on('message', function (message, remote) {
		// console.log("UDP Client: message Rx" + remote.address + ':' + remote.port +' - ' + message);

		var reply = message.toString('utf8')
		socket.emit('commandReply', reply);

		client.close();

	});
}

function updateTempo(socket) {
	var client = dgram.createSocket('udp4');
	var buffer = "tempo";
	client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
		if (err) 
			throw err;
		// console.log('UDP message sent to ' + HOST +':'+ PORT);
	});

	client.on('listening', function () {
		var address = client.address();
		// console.log('UDP Client: listening on ' + address.address + ":" + address.port);
	});
	client.on('message', function (message, remote) {
		// console.log("UDP Client: message Rx" + remote.address + ':' + remote.port +' - ' + message);

		var reply = message.toString('utf8')
		socket.emit('commandReply', reply);

		client.close();

	});
}

function updateMode(socket) {
	var client = dgram.createSocket('udp4');
	var buffer = "curmode";
	client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
		if (err) 
			throw err;
		// console.log('UDP message sent to ' + HOST +':'+ PORT);
	});

	client.on('listening', function () {
		var address = client.address();
		// console.log('UDP Client: listening on ' + address.address + ":" + address.port);
	});
	client.on('message', function (message, remote) {
		// console.log("UDP Client: message Rx" + remote.address + ':' + remote.port +' - ' + message);

		var reply = message.toString('utf8')
		socket.emit('commandReply', reply);

		client.close();

	});
}

function updateStatus(socket) {
	var client = dgram.createSocket('udp4');
	var buffer = "uptime";
	client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
		if (err) 
			throw err;
		// console.log('UDP message sent to ' + HOST +':'+ PORT);
	});

	client.on('listening', function () {
		var address = client.address();
		// console.log('UDP Client: listening on ' + address.address + ":" + address.port);
	});
	var errorTimer = setTimeout(function() {
		var reply = "error beatboxNotRunning".toString('utf8')
		socket.emit("commandReply", reply)
	}, 1000)
	client.on('message', function (message, remote) {
		clearTimeout(errorTimer);

		// console.log("UDP Client: message Rx" + remote.address + ':' + remote.port +' - ' + message);

		var reply = message.toString('utf8')
		socket.emit('commandReply', reply);

		client.close();

	});
}
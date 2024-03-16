"use strict";
// Client-side interactions with the browser.

// Make connection to server when web page is fully loaded.





var socket = io.connect();
var connected;
var intervalId
var errorTimeout
var errorMessages = []


$(document).ready(function() {

	$('#modeNone').click(function(){
		sendCommandViaUDP("mode none");
	});
	$('#modeRock').click(function(){
		sendCommandViaUDP("mode rock");
	});
	$('#modeCustom').click(function(){
		sendCommandViaUDP("mode custom");
	});
	$('#volumeDown').click(function(){
		sendCommandViaUDP("volume down");
	});
	$('#volumeUp').click(function(){
		sendCommandViaUDP("volume up");
	});
	$('#BPMDown').click(function(){
		sendCommandViaUDP("bpm down");
	});
	$('#BPMUp').click(function(){
		sendCommandViaUDP("bpm up");
	});
	$('#stop').click(function(){
		sendCommandViaUDP("stop");
		document.getElementById('status').innerText = "Beat Box is currently stopped"
		document.getElementById('modeid').innerText = ""
		document.getElementById('volume').innerText = "Volume"
		document.getElementById('bpm').innerText = "BPM"
		clearInterval(intervalId)
	});

	$('#kick').click(function(){
		sendCommandViaUDP("play kick");
	});
	$('#hihat').click(function(){
		sendCommandViaUDP("play hihat");
	});
	$('#snare').click(function(){
		sendCommandViaUDP("play snare");
	});
	$('#open_hihat').click(function(){
		sendCommandViaUDP("play open hihat");
	});
	$('#conga').click(function(){
		sendCommandViaUDP("play conga");
	});
	$('#tube').click(function(){
		sendCommandViaUDP("play tube");
	});
	$('#crash').click(function(){
		sendCommandViaUDP("play crash");
	});

	socket.on('connect', () => {
		connected = true
		intervalId = window.setInterval(function(){
			sendCommandViaUDP("uptime");
		}, 1000);
	})

	socket.on('disconnect', () => {
		connected = false;
		clearInterval(intervalId)
		displayError("Node server has shut down")
	})
	
	socket.on('uptime', function(result) {
		document.getElementById('status').innerText = result
	});

	socket.on('mode', function(result) {
		document.getElementById('modeid').innerText = result
	});
	
	socket.on('volume', function(result) {
		document.getElementById('volume').innerText = result
	})

	socket.on('bpm', function(result) {
		document.getElementById('bpm').innerText = result
	})

	socket.on('error', function(result) {
		displayError(result)
	})

});

function displayError(error) {
	document.getElementById('error-box').style.display = "inline"
	errorMessages.push(divMessage("ERROR: " + error))
	console.log("error")
	$('#error-text').empty()
	for(let i = 0; i < errorMessages.length; i++) {
		$('#error-text').append(errorMessages[i])
	}
	clearTimeout(errorTimeout)
	errorTimeout = setTimeout(function() {
		//clearTimeout(errorTimeout)
		document.getElementById('error-box').style.display = "none"
		
	}, 10000)
	setTimeout(function() {
		errorMessages.shift()
		$('#error-text').empty()
		for(let i = 0; i < errorMessages.length; i++) {
			$('#error-text').append(errorMessages[i])
		}
	}, 10000)
	
}


function sendCommandViaUDP(message) {
	if(connected) {
		socket.emit('daUdpCommand', message);
	} else {
		displayError("Node server is down")
	}
	
	// timeout = setTimeout(() => {
	// 	console.log('Event 2 was not triggered within 1 second');
		
	// }, 1000);
	// socket.on('reply', ()=> {
	// 	socket.emit('error', "Node server offline")
	// 	clearTimeout(timeout)
	// })
};

function divMessage(inString) {
	return $('<div></div>').text(inString);
}
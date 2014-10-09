//-----------------------------------------------------------------------------------------------
//Includes and defines
var UDP_PORT = 5000;
var SERVER_IP = '127.0.0.1';
var TIMEOUT_TIME_IN_MILLISECONDS = 5000;
var FREQUENCY_OF_BROADCAST_TO_CLIENTS = 50;

var Class_Map = require( 'collection' ).Map;
var dgram = require( 'dgram' );

//-----------------------------------------------------------------------------------------------
//Global Variables
var udpClients = new Class_Map();
var udpServer = dgram.createSocket( 'udp4' );


//-----------------------------------------------------------------------------------------------
//Classes
var Class_UniqueClient = function( ipAddressAsString, portAsString, timeStamp )
{
	this.ipAddress 				 = ipAddressAsString.toString();
	this.port      				 = portAsString.toString();
	this.lastReceivedMessageTime = timeStamp;
	this.lastReceivedPacket		 = null;
} 


//-----------------------------------------------------------------------------------------------
//Functions
//This will need to actually parse packets later
var ParsePacket = function( buffer )
{
	var out = buffer;
	
	return out;
}


var RemoveAnyTimedOutUDPClients = function()
{
	var time = Date.now();
	var disconnected = [];

	console.log( "\nConnected Clients:" );

	udpClients.each( function( mapIter )
	{
		var client = mapIter.value();
		if( time - mapIter.value().lastReceivedMessageTime > TIMEOUT_TIME_IN_MILLISECONDS )
		{
			disconnected.push( mapIter.key() );
		}
		else
		{
			console.log( 'Client ID= ' + client.port + client.ipAddress  );
		}

	} );

	if( disconnected.length > 0 )
	{
		for( var i = disconnected.length - 1; i >= 0; --i )
		{
			udpClients.remove( disconnected[ i ] );
		}
	}

	if( udpClients.size() === 0 )
	{
		console.log( "None" );
	}

	setTimeout( RemoveAnyTimedOutUDPClients, TIMEOUT_TIME_IN_MILLISECONDS );
}


var UpdateOrAddClient = function( parsedPacket, sender )
{
	var idOfClient = sender.port + sender.address;
	var currentTime = Date.now();

	var existingClientIter = udpClients.find( function( mapIter )
	{ 
		return mapIter.key() === idOfClient; 
	} );

	if( existingClientIter === undefined )
	{	
		var newClient = new Class_UniqueClient( sender.address, sender.port, currentTime );
		newClient.lastReceivedPacket = parsedPacket;
		udpClients.set( idOfClient, newClient );
	}
	else
	{
		existingClientIter.value().lastReceivedPacket = parsedPacket;
		existingClientIter.value().lastReceivedMessageTime = currentTime;
	}
}


var BroadcastClientStateDataToAllClients = function()
{
	udpClients.each( function( clientToSendToIter )
	{
		var clientToSendTo = clientToSendToIter.value();

		udpClients.each( function( otherClientIter )
		{
			var otherClientPacket = otherClientIter.value().lastReceivedPacket;

			if( otherClientPacket != null )
			{
				udpServer.send( otherClientPacket, 0, otherClientPacket.length, clientToSendTo.port, clientToSendTo.ipAddress );
			}
		} );
		
	} );

	setTimeout( BroadcastClientStateDataToAllClients, FREQUENCY_OF_BROADCAST_TO_CLIENTS );
}


//-----------------------------------------------------------------------------------------------
//Event Funcs
udpServer.on( 'message', function( message, sender )
{
	//console.log( '\nPacket received from client with ip address: ' + sender.address + ' and port: ' + sender.port );

	var parsedPacket = ParsePacket( message );

	UpdateOrAddClient( parsedPacket, sender );
	
	if( !parsedPacket )
	{
		//console.log( 'Got UDP Message: ' + ( message + '' ) );
		udpServer.send( message, 0, message.length, sender.port, sender.address );
	}
} );


//-----------------------------------------------------------------------------------------------
//Main
udpServer.bind( UDP_PORT );

setTimeout( BroadcastClientStateDataToAllClients, FREQUENCY_OF_BROADCAST_TO_CLIENTS );
setTimeout( RemoveAnyTimedOutUDPClients, TIMEOUT_TIME_IN_MILLISECONDS );

console.log( "UDP Server Listening on Port: " + UDP_PORT );
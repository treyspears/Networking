#include "CommandRegistry.hpp"

void CommandRegistry::RegisterEvent( const std::string& key, CommandFuncPtr function )
{
	registeredCommands[key] = function;
}

std::map<std::string, CommandFuncPtr>::iterator CommandRegistry::GetCommandRegistryIter(const std::string& key)
{
	return registeredCommands.find(key);
}

void CommandRegistry::FireEvent( const std::string& functionName, ConsoleLog* log )
{
	ConsoleCommandArgs commandArguments(functionName);

	auto foundIter = GetCommandRegistryIter(commandArguments.m_argsList[0] );
	if( foundIter == registeredCommands.end() )
	{
		if( log != nullptr )
		{
			log->ConsolePrint( "Error: " + commandArguments.m_argsList[0] + " could not be found!",Vector4f(1.f,0.f,0.f,1.f), true );
		}
	}
	else
	{
		foundIter->second( commandArguments );
	}
}
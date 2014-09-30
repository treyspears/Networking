#ifndef COMMANDREGISTRY
#define COMMANDREGISTRY

#include <map>
#include <string>

#include "Engine/Rendering/ConsoleLog.hpp"

class ConsoleCommandArgs
{
public:
	ConsoleCommandArgs(const std::string& argsString)
		: m_argsString(argsString)
	{
		std::vector<std::string> args = SplitStringByStringDelimiter(argsString, " ");
		for(unsigned int i = 0; i < args.size(); ++ i)
		{
			m_argsList.push_back(args[i]);
		}
	}

	std::string m_argsString;
	std::vector<std::string> m_argsList;
};

typedef void (*CommandFuncPtr)(const ConsoleCommandArgs& args);

class CommandRegistry
{
public:

	static CommandRegistry& GetInstance()
	{
		static CommandRegistry instance; 

		return instance;
	}

	void RegisterEvent( const std::string& key, CommandFuncPtr function );
	std::map<std::string, CommandFuncPtr>::iterator GetCommandRegistryIter( const std::string& key );
	void FireEvent( const std::string& functionName, ConsoleLog* log );

private:

	CommandRegistry() {}; 

	CommandRegistry( CommandRegistry const& );              
	void operator=( CommandRegistry const& ); 


	std::map<std::string, CommandFuncPtr> registeredCommands;
};







#endif
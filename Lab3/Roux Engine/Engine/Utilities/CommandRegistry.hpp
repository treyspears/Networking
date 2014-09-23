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

static std::map<std::string, CommandFuncPtr> s_registeredCommands;

inline void RegisterEvent(const std::string& key, CommandFuncPtr function)
{
	s_registeredCommands[key] = function;
}

inline std::map<std::string, CommandFuncPtr>::iterator GetCommandRegistryIter(const std::string& key)
{
	return s_registeredCommands.find(key);
}

inline void FireEvent(const std::string& functionName, ConsoleLog* log)
{
	ConsoleCommandArgs commandArguments(functionName);

	auto foundIter = GetCommandRegistryIter(commandArguments.m_argsList[0]);
	if(foundIter == s_registeredCommands.end())
	{
		if(log != nullptr)
		{
			log->ConsolePrint("Error: " + commandArguments.m_argsList[0] + " could not be found!",Vector4f(1.f,0.f,0.f,1.f),true);
		}
	}
	else
	{
		foundIter->second(commandArguments);
	}
}

#endif
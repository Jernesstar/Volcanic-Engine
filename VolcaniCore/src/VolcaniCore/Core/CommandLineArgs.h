#pragma once

#include "Assert.h"

#include "List.h"

namespace VolcaniCore {

struct ArgList {
	bool Valid = false;
	List<std::string> Args;

	operator std::string() const { return Args[0]; }
	operator bool() const { return Valid && Args; }

	List<std::string>::iterator begin() { return Args.begin(); }
	List<std::string>::iterator end() { return Args.end(); }
	List<std::string>::const_iterator begin() const { return Args.cbegin(); }
	List<std::string>::const_iterator end()	const { return Args.cend(); }
};

class CommandLineArgs {
public:
	u32 Count;

	CommandLineArgs(u32 argc, char** argv, bool skipFirst = true)
		: Count(argc - (u32)skipFirst), m_Args(argc - (u32)skipFirst)
	{
		i32 lastOption = -1;
		for(u32 i = 0; i < Count; i++) {
			m_Args.Add(std::string(argv[i + (u32)skipFirst]));
			if(m_Args[i][0] == '-') {
				lastOption = i;
				m_ArgMap[m_Args[lastOption]] = ArgList(true);
			}
			else if(lastOption != -1)
				m_ArgMap[m_Args[lastOption]].Args.Add(m_Args[i]);
		}
	}

	std::string operator [](u32 index) const {
		VOLCANICORE_ASSERT(index < Count);
		return m_Args[index];
	}

	ArgList operator [](const std::string& option) const {
		if(!Has(option))
			return ArgList(false);
		return m_ArgMap.at(option);
	}

	bool Has(const std::string& option) const {
		return m_ArgMap.count(option);
	}

private:
	List<std::string> m_Args;
	std::unordered_map<std::string, ArgList> m_ArgMap;
};

}
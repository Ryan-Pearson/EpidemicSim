//
// Created by Ryan S. Pearson on 4/18/20.
//

#ifndef EPIDEMIC_SIM_AGENT_H
#define EPIDEMIC_SIM_AGENT_H

// STL
#include <string>
#include <unordered_map>

namespace Epidemic {

struct AgentConfiguration
{
   std::string m_name;
};

class Agent
{
public:
   using Id = int;
   using Type = int;

   explicit Agent() noexcept;

   static Type get_agent_type_by_name(const std::string& name);

private:
   Id m_id;

   static std::unordered_map<std::string, Type> s_typeByName;
};

} // namespace Epidemic

#endif // EPIDEMIC_SIM_AGENT_H
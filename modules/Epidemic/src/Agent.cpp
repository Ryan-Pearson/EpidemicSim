#include "Epidemic/Agent.h"

namespace Epidemic {

static Agent::Id uniqueAgentId = -1;
static Agent::Type uniqueAgentType = -1;

Agent::Agent(const AgentConfiguration& agentConfiguration):
   m_id(++uniqueAgentId), m_type(get_agent_type_by_name(agentConfiguration.m_name))
{
}

Agent::Type Agent::get_agent_type_by_name(const std::string& name)
{
   auto it = s_typeByName.find(name);
   if (it == s_typeByName.cend())
   {
      const auto emplace_ret = s_typeByName.emplace(name, ++uniqueAgentType);
      it = emplace_ret.first;
   }
   return it->second;
}

} // namespace Epidemic
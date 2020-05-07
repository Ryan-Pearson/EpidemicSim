//
// Created by Ryan S. Pearson on 4/18/20.
//

#ifndef EPIDEMIC_SIM_AGENT_H
#define EPIDEMIC_SIM_AGENT_H

// STL
#include <string>
#include <unordered_map>

namespace Epidemic {

class Agent
{
public:
   using Id = int;
   using Type = int;

   static Type get_agent_type_by_name(const std::string& name);

   explicit Agent(const std::string& agentName);

   [[nodiscard]] constexpr Id get_id() const noexcept { return m_id; }

private:
   Id m_id;
   Type m_type;

   static std::unordered_map<std::string, Type> s_typeByName;
};

} // namespace Epidemic

#endif // EPIDEMIC_SIM_AGENT_H
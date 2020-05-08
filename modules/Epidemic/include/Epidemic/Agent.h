//
// Created by Ryan S. Pearson on 4/18/20.
//

#ifndef EPIDEMIC_SIM_AGENT_H
#define EPIDEMIC_SIM_AGENT_H

// STL
#include <string>
#include <unordered_map>

// Epidemic
#include "Community.h"
#include "Types.h"

namespace Epidemic {

class Building;

class Agent
{
public:
   static AgentType get_agent_type_by_name(const std::string& name);

   explicit Agent(const std::string& agentName, Building* curBuilding, const Building::Position& curPosition);

   [[nodiscard]] constexpr AgentId get_id() const noexcept { return m_id; }

private:
   AgentId m_id;
   AgentType m_type;
   Building* m_curBuilding;
   Building::Position m_curPosition;

   static std::unordered_map<std::string, AgentType> s_typeByName;
};

} // namespace Epidemic

#endif // EPIDEMIC_SIM_AGENT_H
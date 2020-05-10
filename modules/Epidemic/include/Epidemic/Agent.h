//
// Created by Ryan S. Pearson on 4/18/20.
//

#ifndef EPIDEMIC_SIM_AGENT_H
#define EPIDEMIC_SIM_AGENT_H

// STL
#include <array>
#include <string>
#include <optional>
#include <unordered_map>

// Boost
#include <boost/container/flat_map.hpp>
#include <boost/container/small_vector.hpp>

// Epidemic
#include "Community.h"
#include "Types.h"
#include "SIRDStates.h"

namespace Epidemic {

class Building;

struct AgentMovementInfo
{
   Statistics::Distribution m_locationIdx;
   boost::container::small_vector<Building*, 10> m_locations;
   std::optional<double> m_lambda;
};

class Agent
{
public:
   static AgentType get_agent_type_by_name(const std::string& name);

   explicit Agent(const std::string& agentName, Building* curBuilding, const Building::Position& curPosition);

   [[nodiscard]] constexpr AgentId get_id() const noexcept { return m_id; }
   [[nodiscard]] SIRD get_SIRD_state() const & noexcept { return m_currentState; }
   [[nodiscard]] const SIRD& get_SIRD_state() && noexcept { return m_currentState; }

   void update_agent_state(Timestep curTimeStep);
   void move_agent(Timestep curTimeStep);
   void attempt_infection(Timestep curTimeStep);
   [[nodiscard]] std::vector<AgentId> get_nearby_agents_to_infect() const;

private:
   AgentId m_id;
   AgentType m_type;
   SIRD m_currentState = SirdState::Susceptible{};
   Building* m_curBuilding;
   Timestep m_nextMovementTime = 0;
   Building::Position m_curPosition;
   boost::container::flat_map<Timestep, AgentMovementInfo> m_locations;

   size_t m_nextRandomMovementIdx;
   size_t m_stride;

   static std::unordered_map<std::string, AgentType> s_typeByName;
};

} // namespace Epidemic

#endif // EPIDEMIC_SIM_AGENT_H
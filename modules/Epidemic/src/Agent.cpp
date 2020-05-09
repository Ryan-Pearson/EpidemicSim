#include "Epidemic/Agent.h"

namespace Epidemic {

static AgentId uniqueAgentId = -1;
static AgentType uniqueAgentType = -1;
std::unordered_map<std::string, AgentType> Agent::s_typeByName;

constexpr double MAX_INFECTION_RADIUS = 10.0;

Agent::Agent(const std::string& agentName, Building* curBuilding, const Building::Position& curPosition) :
   m_id(++uniqueAgentId),
   m_type(get_agent_type_by_name(agentName)),
   m_curBuilding(curBuilding),
   m_curPosition(curPosition)
{
   (void)m_curPosition;
   (void)m_curBuilding;
}

AgentType Agent::get_agent_type_by_name(const std::string& name)
{
   auto it = s_typeByName.find(name);
   if (it == s_typeByName.cend())
   {
      const auto emplace_ret = s_typeByName.emplace(name, ++uniqueAgentType);
      it = emplace_ret.first;
   }
   return it->second;
}

std::vector<AgentId> Agent::get_nearby_agents_to_infect() const
{
   std::vector<AgentId> ret;

   // If we don't have the infection, can't spread it
   if (!std::get_if<SirdState::Infectious>(&m_currentState))
   {
      return ret;
   }

   const auto nearbyAgentsAndDistance = m_curBuilding->get_nearby_agents(MAX_INFECTION_RADIUS, m_id);
   for (const auto& curAgentAndDistance : nearbyAgentsAndDistance)
   {
      const AgentId agentToAttemptToInfect = curAgentAndDistance.first;
      const Building::Distance distanceToAgent = curAgentAndDistance.second;

      // TODO: Make better
      static std::normal_distribution infectionDist(0.0, 2.0);
      const double draw = infectionDist(Statistics::GLOBAL_RANDOM_ENGINE);
      const bool infectionSuccessful = std::abs(draw) > distanceToAgent;

      if (infectionSuccessful)
      {
         ret.emplace_back(agentToAttemptToInfect);
      }
   }

   return ret;
}

void Agent::attempt_infection(const Timestep curTimeStep)
{
   if (std::get_if<SirdState::Susceptible>(&m_currentState))
   {
      m_currentState = SirdState::Infectious {curTimeStep + INFECTION_LENGTH};
   }
}

void Agent::update_agent_state(const Timestep curTimeStep)
{
   if (const auto infectious = std::get_if<SirdState::Infectious>(&m_currentState))
   {
      if (infectious->m_infectionEndTime < curTimeStep)
      {
         // We are being generous... for now
         m_currentState = SirdState::Recovered {curTimeStep};
      }
   }
}

void Agent::move_agent()
{
   assert(m_curBuilding != nullptr);

   const auto& maxPosition = m_curBuilding->get_max_position();
   const double distanceToMove = TIMESTEP_TO_TIME_IN_SEC * AGENT_SPEED;

   std::uniform_real_distribution<> randomAngleDistribution(0, 2.0 * M_PI);
   const double randomAngle = randomAngleDistribution(Statistics::GLOBAL_RANDOM_ENGINE);

   const double yDelta = std::sin(randomAngle) * distanceToMove;
   const double xDelta = std::cos(randomAngle) * distanceToMove;

   const double newX = std::clamp(m_curPosition.m_x + xDelta, 0.0, maxPosition.m_x);
   const double newY = std::clamp(m_curPosition.m_y + yDelta, 0.0, maxPosition.m_y);

   m_curPosition = Building::Position {newX, newY};
}

} // namespace Epidemic
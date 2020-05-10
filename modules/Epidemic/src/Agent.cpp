#include "Epidemic/Agent.h"

namespace Epidemic {

static std::vector<std::pair<double, double>> generate_random_movement_vector(const size_t toGenerate)
{
   std::vector<std::pair<double, double>> ret;

   std::uniform_real_distribution<> randomAngleDistribution(0, 2.0 * M_PI);
   ret.reserve(toGenerate);

   for (size_t i = 0; i < toGenerate; ++i)
   {
      const double randomAngle = randomAngleDistribution(Statistics::GLOBAL_RANDOM_ENGINE);
      const double sinX = std::sin(randomAngle);
      const double cosX = std::cos(randomAngle);
      ret.emplace_back(sinX, cosX);
   }

   return ret;
}

static AgentId uniqueAgentId = -1;
static AgentType uniqueAgentType = -1;
constexpr size_t numRandomMovements = 100000;
static const std::vector<std::pair<double, double>> randomMovementVector =
   generate_random_movement_vector(numRandomMovements);
std::unordered_map<std::string, AgentType> Agent::s_typeByName;

constexpr double MAX_INFECTION_RADIUS = 10.0;

Agent::Agent(const std::string& agentName, boost::container::flat_map<Timestep, AgentMovementInfo> locations) :
   m_id(++uniqueAgentId), m_type(get_agent_type_by_name(agentName)), m_locations(std::move(locations))
{
   std::uniform_int_distribution<> randStart(0, numRandomMovements);
   std::uniform_int_distribution<> randStride(0, std::min(50, static_cast<int>(numRandomMovements / 2)));
   m_nextRandomMovementIdx = randStart(Statistics::GLOBAL_RANDOM_ENGINE);
   m_stride = randStride(Statistics::GLOBAL_RANDOM_ENGINE);

   const auto startLocationIt = m_locations.begin();
   const auto nextLocationIdx = Statistics::sample_distribution(startLocationIt->second.m_locationIdx);
   m_curBuilding = startLocationIt->second.m_locations[nextLocationIdx];
   m_curPosition = m_curBuilding->agent_enters_building(m_id);
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

void Agent::move_agent(const Timestep curTimeStep)
{
   assert(m_curBuilding != nullptr);

   if (curTimeStep == m_nextMovementTime)
   {
      const int curHour =
         static_cast<int>(static_cast<double>(curTimeStep % TIMESTEP_PER_DAY) * TIMESTEP_TO_TIME_IN_HOUR + 1e-6);
      const auto locationInfo = m_locations.lower_bound(curHour);
      const auto nextLocationIdx = Statistics::sample_distribution(locationInfo->second.m_locationIdx);

      m_curBuilding->agent_leaves_building(m_id);
      m_curBuilding = locationInfo->second.m_locations[nextLocationIdx];
      m_curPosition = m_curBuilding->agent_enters_building(m_id);

      const auto nextLocationInfoIt = std::next(locationInfo);
      const auto nextLocationTimeInMap = (nextLocationInfoIt != m_locations.cend()) ? nextLocationInfoIt->first : 0;

      if (!locationInfo->second.m_lambda)
      {
         m_nextMovementTime = nextLocationTimeInMap;
      }
      else
      {
         const double lambdaTimestep = *locationInfo->second.m_lambda * TIME_IN_HOUR_TO_TIMESTEP;
         std::exponential_distribution exponentialDistribution(lambdaTimestep);
         const auto numTimestepsExponential =
            static_cast<Timestep>(exponentialDistribution(Statistics::GLOBAL_RANDOM_ENGINE));
         m_nextMovementTime =
            ((numTimestepsExponential > nextLocationTimeInMap) || (numTimestepsExponential >= TIMESTEP_PER_DAY)) ?
            nextLocationTimeInMap :
            numTimestepsExponential;
      }
   }
   else
   {

      const auto& maxPosition = m_curBuilding->get_max_position();
      const double distanceToMove = TIMESTEP_TO_TIME_IN_SEC * AGENT_SPEED;

      const double yDelta = randomMovementVector[m_nextRandomMovementIdx].first * distanceToMove;
      const double xDelta = randomMovementVector[m_nextRandomMovementIdx].second * distanceToMove;
      m_nextRandomMovementIdx = (m_nextRandomMovementIdx + m_stride) % numRandomMovements;

      const double newX = std::clamp(m_curPosition.m_x + xDelta, 0.0, maxPosition.m_x);
      const double newY = std::clamp(m_curPosition.m_y + yDelta, 0.0, maxPosition.m_y);

      m_curPosition = Building::Position {newX, newY};
   }
}

} // namespace Epidemic
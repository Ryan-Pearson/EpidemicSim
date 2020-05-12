#include "Epidemic/Agent.h"

namespace Epidemic {

static std::vector<std::pair<double, double>> generate_random_movement_vector(const size_t toGenerate)
{
   std::vector<std::pair<double, double>> ret;

   std::uniform_real_distribution<> randomAngleDistribution(0.0, 2.0 * M_PI);
   ret.reserve(toGenerate);

   for (size_t i = 0; i < toGenerate; ++i)
   {
      const double randomAngle = randomAngleDistribution(Statistics::get_global_random_engine());
      const double sinX = std::sin(randomAngle);
      const double cosX = std::cos(randomAngle);
      ret.emplace_back(sinX, cosX);
   }

   return ret;
}

thread_local AgentId uniqueAgentId = -1;
thread_local AgentType uniqueAgentType = -1;
constexpr size_t numRandomMovements = 100000;
thread_local const std::vector<std::pair<double, double>> randomMovementVector =
   generate_random_movement_vector(numRandomMovements);
thread_local std::unordered_map<std::string, AgentType> Agent::s_typeByName;

constexpr double MAX_INFECTION_RADIUS = 20.0;

Agent::Agent(Building* spawnLocation,
   const std::string& agentName,
   boost::container::flat_map<Timestep, AgentMovementInfo> locations,
   const Statistics::Distribution infectionSymptomsDist,
   const Statistics::Distribution infectionDurationDist,
   const double mortalityRate) :
   m_id(++uniqueAgentId),
   m_type(get_agent_type_by_name(agentName)),
   m_spawnLocation(spawnLocation),
   m_infectionSymptomsDist(infectionSymptomsDist),
   m_infectionDurationDist(infectionDurationDist),
   m_mortalityRate(mortalityRate),
   m_locations(std::move(locations))
{
   std::uniform_int_distribution<> randStart(0, numRandomMovements);
   std::uniform_int_distribution<> randStride(0, std::min(50, static_cast<int>(numRandomMovements / 2)));
   m_nextRandomMovementIdx = randStart(Statistics::get_global_random_engine());
   m_stride = randStride(Statistics::get_global_random_engine());

   const auto startLocationIt = m_locations.begin();
   const auto nextLocationIdx = Statistics::sample_distribution(startLocationIt->second.m_locationIdx);
   m_curBuilding = startLocationIt->second.m_locations[nextLocationIdx];
   m_curPosition = m_curBuilding->agent_enters_building(m_id);

   const auto nextLocationIt = std::next(startLocationIt);
   m_nextMovementTime = (nextLocationIt != m_locations.cend()) ? nextLocationIt->first : 0;
}

void Agent::reset_static_counters()
{
   uniqueAgentId = -1;
   uniqueAgentType = -1;
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
      thread_local std::normal_distribution infectionDist(0.0, 6.0);
      const double draw = infectionDist(Statistics::get_global_random_engine());
      const bool infectionSuccessful = std::abs(draw) > distanceToAgent;

      if (infectionSuccessful)
      {
         ret.emplace_back(agentToAttemptToInfect);
      }
   }

   return ret;
}

std::optional<Timestep> Agent::attempt_infection(const Timestep curTimeStep)
{
   if (std::get_if<SirdState::Susceptible>(&m_currentState))
   {
      const int durationDays = std::max(2, Statistics::sample_distribution(m_infectionDurationDist));
      const int symptomsDays = std::min(durationDays - 1, Statistics::sample_distribution(m_infectionSymptomsDist));

      const Timestep duration = durationDays * TIMESTEP_PER_DAY;
      const Timestep symptoms = symptomsDays * TIMESTEP_PER_DAY;

      m_currentState = SirdState::Infectious {curTimeStep + symptoms, curTimeStep + duration};
      return duration;
   }
   else
   {
      return std::nullopt;
   }
}

void Agent::update_agent_state(const Timestep curTimeStep)
{
   if (const auto infectious = std::get_if<SirdState::Infectious>(&m_currentState))
   {
      if (!m_quarantined)
      {
         m_quarantined = true;
      }
      else if (infectious->m_infectionEndTime < curTimeStep)
      {
         std::uniform_real_distribution<> pkDraw(0.0, 1.0);
         const double draw = pkDraw(Statistics::get_global_random_engine());
         m_quarantined = false;

         if (draw < m_mortalityRate)
         {
            m_currentState = SirdState::Deceased {curTimeStep};
         }
         else
         {
            m_currentState = SirdState::Recovered {curTimeStep};
         }
      }
   }
}

void Agent::move_agent(const Timestep curTimeStep)
{
   assert(m_curBuilding != nullptr);

   const auto timeOfDay = (curTimeStep % TIMESTEP_PER_DAY);

   if (m_quarantined && m_curBuilding != m_spawnLocation)
   {
      if (const auto infectious = std::get_if<SirdState::Infectious>(&m_currentState))
      {
         m_curBuilding->agent_leaves_building(m_id);
         m_curBuilding = m_spawnLocation;
         m_curPosition = m_curBuilding->agent_enters_building(m_id);
         const auto endInfectionTod = (infectious->m_infectionEndTime % TIMESTEP_PER_DAY);
         const auto nextLocation = m_locations.lower_bound(endInfectionTod);
         m_nextMovementTime = (nextLocation == m_locations.cend()) ? 0 : nextLocation->first;
      }
      else
      {
         throw std::runtime_error("Quarantined but not infectious, how did we get here?");
      }
   }
   else if (timeOfDay == m_nextMovementTime)
   {
      const auto locationInfo = m_locations.lower_bound(timeOfDay);
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
         std::exponential_distribution exponentialDistribution(1.0 / lambdaTimestep);
         const auto exponentialDraw = exponentialDistribution(Statistics::get_global_random_engine());
         const auto numTimestepsExponential = std::max(1, static_cast<Timestep>(exponentialDraw));
         const auto nextExponentialTime = timeOfDay + numTimestepsExponential;
         m_nextMovementTime =
            ((nextExponentialTime > nextLocationTimeInMap) || (nextExponentialTime >= TIMESTEP_PER_DAY)) ?
            nextLocationTimeInMap :
            nextExponentialTime;
      }
   }
   else
   {

      const auto& maxPosition = m_curBuilding->get_max_position();
      const double distanceToMove = TIMESTEP_TO_TIME_IN_SEC * AGENT_SPEED;

      const auto nextMovePair = randomMovementVector[m_nextRandomMovementIdx];
      const double yDelta = nextMovePair.first * distanceToMove;
      const double xDelta = nextMovePair.second * distanceToMove;
      m_nextRandomMovementIdx = (m_nextRandomMovementIdx + m_stride) % numRandomMovements;

      const double newX = std::clamp(m_curPosition.m_x + xDelta, 0.0, maxPosition.m_x);
      const double newY = std::clamp(m_curPosition.m_y + yDelta, 0.0, maxPosition.m_y);

      m_curPosition = Building::Position {newX, newY};
   }
}

} // namespace Epidemic
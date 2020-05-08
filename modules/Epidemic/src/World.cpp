#include "Epidemic/World.h"

namespace Epidemic {

World::World(std::unordered_map<CommunityId, Community> communities, std::unordered_map<AgentId, Agent> agents) :
   m_communities(std::move(communities)), m_agents(std::move(agents))
{
}

} // namespace Epidemic
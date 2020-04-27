#include "Epidemic/Agent.h"

namespace Epidemic {

static int uniqueAgentId = -1;

Agent::Agent() noexcept : m_id(++uniqueAgentId) { }

} // namespace Epidemic
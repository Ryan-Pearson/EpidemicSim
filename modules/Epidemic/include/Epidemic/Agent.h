//
// Created by Ryan S. Pearson on 4/18/20.
//

#ifndef EPIDEMIC_SIM_AGENT_H
#define EPIDEMIC_SIM_AGENT_H

namespace Epidemic {

using AgentID = int;

class Agent
{
public:
   explicit Agent() noexcept;

private:
   Agent m_id;
};

} // namespace Epidemic

#endif // EPIDEMIC_SIM_AGENT_H
//
// Created by Ryan S. Pearson on 5/3/20.
//

#ifndef EPIDEMIC_SIM_XMLREADER_H
#define EPIDEMIC_SIM_XMLREADER_H

// Epidemic
#include "WorldBuilder.h"

namespace Epidemic {

WorldConfiguration get_world_config_from_xml(const std::string& fullFilePath);

} // namespace Epidemic

#endif //EPIDEMIC_SIM_XMLREADER_H

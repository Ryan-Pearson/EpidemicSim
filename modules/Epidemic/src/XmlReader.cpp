//
// Created by Ryan S. Pearson on 5/3/20.
//

#include "Epidemic/XmlReader.h"

// pugixml
#include "pugixml.hpp"

// fmt
#include "fmt/format.h"

// Boost
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace Epidemic {

constexpr char CONFIG_ROOT[] = "EpidemicSimulationConfig";

constexpr char AGENTS_ROOT[] = "Agents";
constexpr char AGENT_NODE[] = "Agent";

constexpr char BUILDING_ROOT[] = "Buildings";
constexpr char BUILDING_NODE[] = "Building";

constexpr char COMMUNITY_ROOT[] = "Communities";
constexpr char COMMUNITY_NODE[] = "Community";

constexpr char WORLD_ROOT[] = "WorldConfiguration";

static Epidemic::Statistics::Distribution read_distribution_from_node(const pugi::xml_node node)
{
   if (const auto pdfNode = node.child("PDF"))
   {
      const auto stringIsEmpty = [](const std::string& toCheck) { return toCheck.empty(); };
      std::vector<std::string> entries;
      boost::algorithm::split(entries, pdfNode.child_value(), boost::is_any_of("\n\r\t "), boost::token_compress_on);
      entries.erase(std::remove_if(entries.begin(), entries.end(), stringIsEmpty), entries.cend());

      if (entries.size() % 2 != 0)
      {
         throw std::runtime_error(
            fmt::format("Read in pdf with {} entries, entries must be divisible by 2", entries.size()));
      }

      boost::container::flat_map<int, int> numberToWeightMap;
      for (size_t i = 0; i < entries.size(); i += 2)
      {
         const int number = boost::lexical_cast<int>(entries[i]);
         const int weight = boost::lexical_cast<int>(entries[i + 1]);
         const auto emplaceRet = numberToWeightMap.emplace(number, weight);

         if (!emplaceRet.second)
         {
            throw std::runtime_error(fmt::format("Error creating valid PDF values for node {}", node.name()));
         }
      }

      return Epidemic::Statistics::PDF(numberToWeightMap);
   }
   else if (const auto gaussianNode = node.child("Gaussian"))
   {
      const auto meanNode = gaussianNode.child("Mean");
      const auto stdDevNode = gaussianNode.child("StdDev");
      if (!meanNode || !stdDevNode)
      {
         throw std::runtime_error(fmt::format("Invalid Gaussian Distribution node at {}", node.name()));
      }

      std::string meanText = meanNode.child_value();
      std::string stdDevText = stdDevNode.child_value();

      boost::algorithm::trim(meanText);
      boost::algorithm::trim(stdDevText);

      const auto mean = boost::lexical_cast<double>(meanText);
      const auto stdDev = boost::lexical_cast<double>(stdDevText);

      return Epidemic::Statistics::Gaussian(mean, stdDev);
   }
   else if (const auto fixedNode = node.child("Fixed"))
   {
      std::string fixedText = fixedNode.child_value();
      boost::algorithm::trim(fixedText);
      return Epidemic::Statistics::Fixed {boost::lexical_cast<int>(fixedText)};
   }
   else
   {
      throw std::runtime_error(fmt::format("Could not find expected distribution for node {}", node.name()));
   }
}

static std::pair<Timestep, AgentMovementConfiguration> get_location_info_from_node(const pugi::xml_node node)
{
   const std::string startTimeStr = node.attribute("start").value();
   const auto startTime =
      static_cast<Timestep>(boost::lexical_cast<double>(startTimeStr) * TIME_IN_HOUR_TO_TIMESTEP + 1e-6);
   if (strcmp(node.name(), "Fixed") == 0)
   {
      std::string fixedText = node.child_value();
      boost::algorithm::trim(fixedText);
      return std::make_pair(startTime,
         AgentMovementConfiguration {Epidemic::Statistics::Fixed {0},
            boost::container::small_vector<std::string, 10> {fixedText}, std::nullopt});
   }
   else if (strcmp(node.name(), "PDF") == 0)
   {
      const auto lambdaAttribute = node.attribute("lambda");
      const std::optional<double> lambda =
         lambdaAttribute ? std::optional<double> {boost::lexical_cast<double>(lambdaAttribute.value())} : std::nullopt;

      const auto stringIsEmpty = [](const std::string& toCheck) { return toCheck.empty(); };
      std::vector<std::string> entries;
      boost::algorithm::split(entries, node.child_value(), boost::is_any_of("\n\r\t "), boost::token_compress_on);
      entries.erase(std::remove_if(entries.begin(), entries.end(), stringIsEmpty), entries.cend());

      if (entries.size() % 2 != 0)
      {
         throw std::runtime_error(
            fmt::format("Read in pdf with {} entries, entries must be divisible by 2", entries.size()));
      }

      boost::container::flat_map<int, int> numberToWeightMap;
      boost::container::small_vector<std::string, 10> locations;
      locations.reserve(entries.size() / 2);
      numberToWeightMap.reserve(entries.size() / 2);
      for (size_t i = 0; i < entries.size(); i += 2)
      {
         locations.emplace_back(entries[i]);
         const int weight = boost::lexical_cast<int>(entries[i + 1]);
         const auto emplaceRet = numberToWeightMap.emplace(i / 2, weight);

         if (!emplaceRet.second)
         {
            throw std::runtime_error(fmt::format("Error creating valid PDF values for node {}", node.name()));
         }
      }
      return std::make_pair(startTime,
         AgentMovementConfiguration {Epidemic::Statistics::PDF {numberToWeightMap}, std::move(locations), lambda});
   }
   else
   {
      throw std::runtime_error("Invalid location node format");
   }
}

static std::unordered_map<AgentId, AgentConfiguration> read_agent_configs(const pugi::xml_node agentsNode)
{
   std::unordered_map<AgentId, AgentConfiguration> ret;

   for (pugi::xml_node agent = agentsNode.child(AGENT_NODE); agent; agent = agent.next_sibling(AGENT_NODE))
   {
      const pugi::xml_attribute agentNameAttribute = agent.attribute("name");
      const pugi::xml_attribute mortalityRateAttribute = agent.attribute("mortality_rate");

      if (!agentNameAttribute || !mortalityRateAttribute)
      {
         throw std::runtime_error("Agents must have valid name and mortality_rate attributes");
      }

      const std::string agentName = agentNameAttribute.value();
      const auto mortalityRate = boost::lexical_cast<double>(mortalityRateAttribute.value());

      const pugi::xml_node locationsNode = agent.child("Locations");
      if (!locationsNode)
      {
         throw std::runtime_error("Could not find locations for agent");
      }

      const pugi::xml_node regularLocations = locationsNode.child("Regular");
      if (!regularLocations)
      {
         throw std::runtime_error("Could not find regular locations for agent");
      }

      boost::container::flat_map<Timestep, AgentMovementConfiguration> locationMap;
      const auto locations = regularLocations.children();
      for (const auto location : locations)
      {
         locationMap.insert(get_location_info_from_node(location));
      }

      ret.emplace(Agent::get_agent_type_by_name(agentName),
         AgentConfiguration {agentName, mortalityRate, std::move(locationMap)});
   }

   return ret;
}

static std::unordered_map<BuildingId, BuildingConfiguration> read_building_configs(const pugi::xml_node buildingsNode)
{
   std::unordered_map<BuildingId, BuildingConfiguration> ret;

   for (pugi::xml_node building = buildingsNode.child(BUILDING_NODE); building;
        building = building.next_sibling(BUILDING_NODE))
   {
      const std::string buildingName = building.attribute("name").value();
      const auto minSize = building.child("MinSize");
      const auto maxSize = building.child("MaxSize");
      if (!minSize || !maxSize)
      {
         throw std::runtime_error(fmt::format("Building {}: Min or Max size not found", buildingName));
      }

      const auto minXNode = minSize.child("x");
      const auto minYNode = minSize.child("y");
      const auto maxXNode = maxSize.child("x");
      const auto maxYNode = maxSize.child("y");
      if (!minXNode || !minYNode || !maxXNode || !maxYNode)
      {
         throw std::runtime_error(fmt::format("Building {}: Min or Max size not properly formatted", buildingName));
      }

      std::string minXStr = minXNode.child_value();
      std::string minYStr = minYNode.child_value();
      std::string maxXStr = maxXNode.child_value();
      std::string maxYStr = maxYNode.child_value();

      boost::algorithm::trim(minXStr);
      boost::algorithm::trim(minYStr);
      boost::algorithm::trim(maxXStr);
      boost::algorithm::trim(maxYStr);

      const auto minX = boost::lexical_cast<double>(minXStr);
      const auto minY = boost::lexical_cast<double>(minYStr);
      const auto maxX = boost::lexical_cast<double>(maxXStr);
      const auto maxY = boost::lexical_cast<double>(maxYStr);

      std::unordered_map<AgentId, Statistics::Distribution> startingAgents;
      const auto startingAgentNodes = building.child("StartingAgents");
      if (startingAgentNodes)
      {
         for (pugi::xml_node startingAgent = startingAgentNodes.child("Agent"); startingAgent;
              startingAgent = startingAgent.next_sibling("Agent"))
         {
            const std::string agentName = startingAgent.attribute("name").value();
            const auto distribution = read_distribution_from_node(startingAgent);
            startingAgents.emplace(Agent::get_agent_type_by_name(agentName), distribution);
         }
      }

      ret.emplace(Building::get_building_type_by_name(buildingName),
         BuildingConfiguration {buildingName, minX, minY, maxX, maxY, std::move(startingAgents)});
   }

   return ret;
}

static std::unordered_map<CommunityId, CommunityConfiguration> read_community_configs(
   const pugi::xml_node communitiesNode)
{
   std::unordered_map<CommunityId, CommunityConfiguration> ret;

   for (pugi::xml_node community = communitiesNode.child(COMMUNITY_NODE); community;
        community = community.next_sibling(COMMUNITY_NODE))
   {
      const std::string communityName = community.attribute("name").value();
      std::unordered_map<BuildingId, Statistics::Distribution> buildings;
      for (pugi::xml_node buildingNode = community.child("Building"); buildingNode;
           buildingNode = buildingNode.next_sibling("Building"))
      {
         const std::string buildingName = buildingNode.attribute("name").value();
         const auto distribution = read_distribution_from_node(buildingNode);
         buildings.emplace(Building::get_building_type_by_name(buildingName), distribution);
      }
      ret.emplace(Community::get_community_type_by_name(communityName),
         CommunityConfiguration {communityName, std::move(buildings)});
   }

   return ret;
}

static std::unordered_map<CommunityId, Statistics::Distribution> read_communinities(const pugi::xml_node worldNode)
{
   std::unordered_map<CommunityId, Statistics::Distribution> ret;

   for (pugi::xml_node community = worldNode.child(COMMUNITY_NODE); community;
        community = community.next_sibling(COMMUNITY_NODE))
   {
      const std::string communityName = community.attribute("name").value();
      ret.emplace(Community::get_community_type_by_name(communityName), read_distribution_from_node(community));
   }

   return ret;
}

WorldConfiguration get_world_config_from_xml(const std::string& fullFilePath)
{
   WorldConfiguration ret;

   pugi::xml_document doc;
   const pugi::xml_parse_result loadResult = doc.load_file(fullFilePath.c_str());
   if (!loadResult)
   {
      throw std::runtime_error(
         fmt::format("Error loading configuration file, xml error description: {}", loadResult.description()));
   }

   const auto mainConfig = doc.child(CONFIG_ROOT);
   if (!mainConfig)
   {
      throw std::runtime_error(fmt::format("Could not find XML Element ", CONFIG_ROOT));
   }

   const auto agentConfigNode = mainConfig.child(AGENTS_ROOT);
   if (agentConfigNode)
   {
      ret.m_agentConfigs = read_agent_configs(agentConfigNode);
   }
   else
   {
      throw std::runtime_error(fmt::format("Could not find XML Element ", AGENTS_ROOT));
   }

   const auto buildingConfigNode = mainConfig.child(BUILDING_ROOT);
   if (buildingConfigNode)
   {
      ret.m_buildingConfigs = read_building_configs(buildingConfigNode);
   }
   else
   {
      throw std::runtime_error(fmt::format("Could not find XML Element ", BUILDING_ROOT));
   }

   const auto communityConfigNode = mainConfig.child(COMMUNITY_ROOT);
   if (communityConfigNode)
   {
      ret.m_communityConfigs = read_community_configs(communityConfigNode);
   }
   else
   {
      throw std::runtime_error(fmt::format("Could not find XML Element ", COMMUNITY_ROOT));
   }

   const auto worldConfigNode = mainConfig.child(WORLD_ROOT);
   if (worldConfigNode)
   {
      ret.m_communities = read_communinities(worldConfigNode);
   }
   else
   {
      throw std::runtime_error(fmt::format("Could not find XML Element ", WORLD_ROOT));
   }

   const auto infectionSize = worldConfigNode.child("InitialInfectionSize");
   if (infectionSize)
   {
      std::string infectionText = infectionSize.child_value();
      boost::algorithm::trim(infectionText);
      ret.m_initialInfectionSize = boost::lexical_cast<int>(infectionText);
   }
   else
   {
      throw std::runtime_error("Could not find XML Element InitialInfectionSize");
   }

   const auto infectionSymptomsLambda = worldConfigNode.child("InfectionSymptomsShow");
   if (infectionSymptomsLambda)
   {
      ret.m_infectionSymptomsDist = read_distribution_from_node(infectionSymptomsLambda);
   }
   else
   {
      throw std::runtime_error("Could not find XML Element InfectionSymptomsShow");
   }

   const auto infectionDurationLambda = worldConfigNode.child("InfectionDuration");
   if (infectionDurationLambda)
   {
      ret.m_infectionDurationDist = read_distribution_from_node(infectionDurationLambda);
   }
   else
   {
      throw std::runtime_error("Could not find XML Element InfectionDuration");
   }

   return ret;
}

} // namespace Epidemic

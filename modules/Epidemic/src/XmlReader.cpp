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

constexpr char COMMUNITY_ROOT[] = "Communities";
constexpr char COMMUNITY_NODE[] = "Community";

constexpr char BUILDING_ROOT[] = "Buildings";
constexpr char BUILDING_NODE[] = "Building";

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
      const auto fixedValue = boost::lexical_cast<int>(fixedText);
      return Epidemic::Statistics::Fixed(fixedValue);
   }
   else
   {
      throw std::runtime_error(fmt::format("Could not find expected distribution for node {}", node.name()));
   }
}

static std::vector<AgentConfiguration> read_agents(const pugi::xml_node agentsNode)
{
   std::vector<AgentConfiguration> ret;

   for (pugi::xml_node agent = agentsNode.child(AGENT_NODE); agent; agent = agent.next_sibling(AGENT_NODE))
   {
      const std::string name = agent.attribute("name").value();
      ret.emplace_back(AgentConfiguration {name});
   }

   return ret;
}

static std::vector<BuildingConfiguration> read_buildings(const pugi::xml_node buildingsNode)
{
   std::vector<BuildingConfiguration> ret;

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

      std::vector<std::pair<std::string, Statistics::Distribution>> startingAgents;
      const auto startingAgentNodes = building.child("StartingAgents");
      if (startingAgentNodes)
      {
         for (pugi::xml_node startingAgent = startingAgentNodes.child("Agent"); startingAgent;
              startingAgent = startingAgent.next_sibling("Agent"))
         {
            const std::string agentName = startingAgent.attribute("name").value();
            const auto distribution = read_distribution_from_node(startingAgent);
            startingAgents.emplace_back(agentName, distribution);
         }
      }

      ret.emplace_back(BuildingConfiguration {buildingName, minX, minY, maxX, maxY, std::move(startingAgents)});
   }

   return ret;
}

static std::vector<CommunityConfiguration> read_communities(const pugi::xml_node communitiesNode)
{
   std::vector<CommunityConfiguration> ret;

   for (pugi::xml_node community = communitiesNode.child(COMMUNITY_NODE); community;
        community = community.next_sibling(COMMUNITY_NODE))
   {
      const std::string communityName = community.attribute("name").value();
      std::unordered_map<std::string, Statistics::Distribution> buildings;
      for (pugi::xml_node buildingNode = community.child("Building"); buildingNode;
           buildingNode = buildingNode.next_sibling("Building"))
      {
         const std::string buildingName = buildingNode.attribute("name").value();
         const auto distribution = read_distribution_from_node(buildingNode);
         buildings.emplace(buildingName, distribution);
      }
      ret.emplace_back(CommunityConfiguration {communityName, std::move(buildings)});
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

   const auto agentsNode = mainConfig.child(AGENTS_ROOT);
   if (agentsNode)
   {
      ret.m_agents = read_agents(agentsNode);
   }
   else
   {
      throw std::runtime_error(fmt::format("Could not find XML Element ", AGENTS_ROOT));
   }

   const auto buildingsNode = mainConfig.child(BUILDING_ROOT);
   if (buildingsNode)
   {
      ret.m_buildings = read_buildings(buildingsNode);
   }
   else
   {
      throw std::runtime_error(fmt::format("Could not find XML Element ", BUILDING_ROOT));
   }

   const auto communitiesNode = mainConfig.child(COMMUNITY_ROOT);
   if (communitiesNode)
   {
      ret.m_communities = read_communities(communitiesNode);
   }
   else
   {
      throw std::runtime_error(fmt::format("Could not find XML Element ", COMMUNITY_ROOT));
   }

   return ret;
}

} // namespace Epidemic

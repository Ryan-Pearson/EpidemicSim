// STL
#include <iostream>

// Boost
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

// fmt
#include "fmt/format.h"

// pugixml
#include "pugixml.hpp"

// Epidemic
#include "Epidemic/CommunityConfiguration.h"
#include "Epidemic/World.h"

constexpr char CONFIG_ROOT[] = "EpidemicSimulationConfig";

constexpr char AGENTS_ROOT[] = "Agents";
constexpr char AGENT_NODE[] = "Agent";

constexpr char COMMUNITY_ROOT[] = "Communities";
constexpr char COMMUNITY_NODE[] = "Community";

constexpr char BUILDING_NODE[] = "Building";

Epidemic::Statistics::Distribution readDistributionFromNode(const pugi::xml_node node)
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
      for (int i = 0; i < entries.size(); i += 2)
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

int main()
{
   const std::string fullFilePath = "/Users/pearsrs1/EpidemicSim/test_input.xml";

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

   const auto agents = mainConfig.child(AGENTS_ROOT);
   if (!agents)
   {
      throw std::runtime_error(fmt::format("Could not find XML Element ", AGENTS_ROOT));
   }

   for (pugi::xml_node agent = agents.child(AGENT_NODE); agent; agent = agent.next_sibling(AGENT_NODE))
   {
      std::cout << "Agent name: " << agent.attribute("name").value() << '\n';
   }

   const auto communities = mainConfig.child(COMMUNITY_ROOT);
   if (!communities)
   {
      throw std::runtime_error(fmt::format("Could not find XML Element ", COMMUNITY_ROOT));
   }

   for (pugi::xml_node community = communities.child(COMMUNITY_NODE); community;
        community = community.next_sibling(COMMUNITY_NODE))
   {
      Epidemic::CommunityConfiguration communityConfiguration(community.attribute("name").value());

      for (pugi::xml_node building = community.child(BUILDING_NODE); building;
           building = building.next_sibling(BUILDING_NODE))
      {
         communityConfiguration.m_buildings.emplace(
            building.attribute("name").value(), readDistributionFromNode(building));
      }
   }

   std::cout << "Got to the end!" << std::endl;

   return EXIT_SUCCESS;
}
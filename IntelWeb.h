#ifndef INTELWEB_H_
#define INTELWEB_H_

#include "DiskMultiMap.h"
#include "InteractionTuple.h"
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>

class IntelWeb
{
public:
	IntelWeb();
	~IntelWeb();
	bool createNew(const std::string& filePrefix, unsigned int maxDataItems);
	bool openExisting(const std::string& filePrefix);
	void close();
	bool ingest(const std::string& telemetryFile);
	unsigned int crawl(const std::vector<std::string>& indicators,
		unsigned int minPrevalenceToBeGood,
		std::vector<std::string>& badEntitiesFound,
		std::vector<InteractionTuple>& badInteractions
		);
	bool purge(const std::string& entity);

private:
	DiskMultiMap forwardMap;
	DiskMultiMap reverseMap;
	map<string, set<string>> InteractionMap;
	map<string, int> PrevalenceMap;
	
	void findInteractions(std::string toSearch);
	int findPrevalences(std::string toSearch);
};

#endif // INTELWEB_H_

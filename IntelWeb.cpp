#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include "IntelWeb.h"
#include "DiskMultiMap.h"

using namespace std;

IntelWeb::IntelWeb()
{}

IntelWeb::~IntelWeb()
{}

bool IntelWeb::createNew(const std::string& filePrefix, unsigned int maxDataItems)
{
	int numBuckets = maxDataItems / 0.75;

	bool success = forwardMap.createNew(filePrefix + "forward", numBuckets);
	if (!success)
		return false;
	success = reverseMap.createNew(filePrefix + "reverse", numBuckets);
	if (!success)
		return false;
}

bool IntelWeb::openExisting(const std::string& filePrefix)
{
	close();

	bool success = forwardMap.openExisting(filePrefix + "forward");
	if (!success)
		return false;
	success = reverseMap.openExisting(filePrefix + "reverse");
	if (!success)
		return false;
}

void IntelWeb::close()
{
	forwardMap.close();
	reverseMap.close();
}

bool IntelWeb::ingest(const std::string& telemetryFile)
{
	ifstream inf(telemetryFile);
	// Test for failure to open
	if (!inf)
	{
		cout << "Cannot open expenses file!" << endl;
		return false;
	}

	string line;
	while (getline(inf, line))
	{
		// To extract the information from the line, we'll
		// create an input stringstream from it, which acts
		// like a source of input for operator>>
		istringstream iss(line);
		string key;
		string value;
		string context;
		// The return value of operator>> acts like false
		// if we can't extract a word followed by a number
		if (!(iss >> context >> key >> value))
		{
			cout << "Ignoring badly-formatted input line: " << line << endl;
			continue;
		}
		// If we want to be sure there are no other non-whitespace
		// characters on the line, we can try to continue reading
		// from the stringstream; if it succeeds, extra stuff
		// is after the double.
		char dummy;
		if (iss >> dummy) // succeeds if there a non-whitespace char
			cout << "Ignoring extra data in line: " << line << endl;

		// Add data to expenses map
		forwardMap.insert(key, value, context);
		reverseMap.insert(value, key, context);

		map<string, int>::iterator it1 = PrevalenceMap.find(key);  //update the PrevalenceMap for the key and value strings
		if (it1 == PrevalenceMap.end())
			PrevalenceMap[it1->first] = 0;
		PrevalenceMap[it1->first]++;
		map<string, int>::iterator it2 = PrevalenceMap.find(value);
		if (it1 == PrevalenceMap.end())
			PrevalenceMap[it1->first] = 0;
		PrevalenceMap[it1->first]++;

		InteractionTuple currTuple;
		currTuple.context = context;
		currTuple.from = key;
		currTuple.to = value;
		InteractionSet.insert(currTuple);
	}
	return true;
}

unsigned int IntelWeb::crawl(const std::vector<std::string>& indicators,
	unsigned int minPrevalenceToBeGood,
	std::vector<std::string>& badEntitiesFound,
	std::vector<InteractionTuple>& badInteractions
	)
{
	std::queue<string> toSearch;

	for (int i = 0; i < indicators.size(); i++)  //take all the known bad entities and push them into a queue toSearch
	{
		toSearch.push(indicators[i]);
		allBadEntities.insert(indicators[i]);  //also insert them into allBadEntities
	}

	while (!toSearch.empty())  //while toSearch contains strings
	{
		string searchingFor = toSearch.front();
		toSearch.pop();
		findInteractions(searchingFor);  //map the set of entities that interact with toSearch.front
		set<string>::iterator it;  //search through every interactor entity
		it = InteractionMap[searchingFor].begin();

		while (it != InteractionMap[searchingFor].end())
		{
			string interactedWith = *it;
			findInteractions(interactedWith);  //map every interactor's own set of entities it interacts with to it in InteractionMap
			set<string> interactors = InteractionMap[interactedWith];  //gets the set of interactors for that entity
			if (PrevalenceMap[interactedWith] < minPrevalenceToBeGood)  //if that interactor has a prevalence less than the prevalence threshold, it is malicious
			{
				set<string>::iterator begin, end, iter;
				begin = allBadEntities.begin();
				end = allBadEntities.end();
				iter = find(begin, end, interactedWith);  //look for the interactor in badEntitiesFound to check if it has already been found
				
				if (iter == end)  //if it has not been found already
				{
					badEntitiesFound.push_back(interactedWith);  //add it to badEntitiesFound
					allBadEntities.insert(interactedWith);  //add it to allBadEntities
					toSearch.push(interactedWith);  //add it to the toSearch queue
				}
			}
		}
	}
	//after searching through all the entities, time to search through all the interactions
	set<InteractionTuple>::iterator it;
	set<string>::iterator it2;
	it = InteractionSet.begin();
	it2 = allBadEntities.begin();
	for (int i = 0; i < allBadEntities.size(); i++)
	{
		InteractionTuple currTuple = *it;
		string toSearch = *it2;
		
		if (currTuple.from == toSearch || currTuple.to == toSearch);
		{
			badInteractions.push_back(currTuple);
		}
	}
	return badEntitiesFound.size();
}

bool IntelWeb::purge(const std::string& entity)
{
	bool result = false;

	DiskMultiMap::Iterator forwardIt = forwardMap.search(entity);
	DiskMultiMap::Iterator reverseIt = reverseMap.search(entity);

	while (forwardIt.isValid())
	{
		MultiMapTuple fTuple = *forwardIt;
		bool fsuccess = forwardMap.erase(fTuple.key, fTuple.value, fTuple.context);
		if (fsuccess)
			result = true;
	}
	while (reverseIt.isValid())
	{
		MultiMapTuple rTuple = *reverseIt;
		bool rsuccess = reverseMap.erase(rTuple.key, rTuple.value, rTuple.context);
		if (rsuccess)
			result = true;
	}
	return result;
}


void IntelWeb::findInteractions(string toSearch)
{
	map<string, set<string>>::iterator it;
	it = InteractionMap.find(toSearch);  //first see if toSearch is already in the InteractionMap

	if (it == InteractionMap.end())  //if it isn't,
	{
		int count;
		set<string> result;
		DiskMultiMap::Iterator itForward = forwardMap.search(toSearch);  //find all the Iterator to the Nodes that have toSearch as a key
		DiskMultiMap::Iterator itReverse = reverseMap.search(toSearch);

		while (itForward.isValid())  //iterate through all Nodes itForward passes through
		{
			MultiMapTuple currNode = *itForward;  //Grab the tuple at the Node
			string value = currNode.value;  //take the value string (what the key interacted with)
			result.insert(value);  //insert it into the set result
			++itForward;
		}
		while (itReverse.isValid())  //do the same thing for reverseMap
		{
			MultiMapTuple currNode = *itReverse;
			string value = currNode.value;
			result.insert(value);
			++itReverse;
		}
		InteractionMap[toSearch] = result;  //map the resulting set of interactors to that string
	}
}



int IntelWeb::findPrevalences(string toSearch)
{
	int count = 0;

	DiskMultiMap::Iterator itForward = forwardMap.search(toSearch);  //find all the Iterator to the Nodes that have toSearch as a key
	DiskMultiMap::Iterator itReverse = reverseMap.search(toSearch);

	while (itForward.isValid())  //iterate through all Nodes itForward passes through
	{
		count++;
		++itForward;
	}
	while (itReverse.isValid())  //do the same thing for reverseMap
	{
		count++;
		++itReverse;
	}
	return count;
}

bool operator<(const InteractionTuple &a, const InteractionTuple &b)
{
	if (a.context != b.context)
		return a.context < b.context;
	else if (a.from != b.from)
		return a.from < b.from;
	else
		return a.to < b.to;
}

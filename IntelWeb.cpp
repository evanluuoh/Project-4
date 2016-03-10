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

		PrevalenceMap.insert(std::pair<string, int>(key, 0));
		PrevalenceMap.insert(std::pair<string, int>(value, 0));
		PrevalenceMap[key]++;
		PrevalenceMap[value]++;

		InteractionTuple currTuple;
		currTuple.context = context;
		currTuple.from = key;
		currTuple.to = value;
		InteractionSet.insert(currTuple);
	}
	set<InteractionTuple>::iterator it3;
	for (it3 = InteractionSet.begin(); it3 != InteractionSet.end(); ++it3)
	{
		InteractionTuple t = *it3;
		cout << t.context << endl;
		cout << t.from << endl;
		cout << t.to << endl;
	}
	return true;
}

unsigned int IntelWeb::crawl(const std::vector<std::string>& indicators,
	unsigned int minPrevalenceToBeGood,
	std::vector<std::string>& badEntitiesFound,
	std::vector<InteractionTuple>& badInteractions
	)
{
	cout << InteractionSet.size() << endl;
	cout << "a" << endl;
	std::queue<string> toSearch;

	for (int i = 0; i < indicators.size(); i++)  //take all the known bad entities and push them into a queue toSearch
	{
		toSearch.push(indicators[i]);
		allBadEntities.insert(indicators[i]);  //also insert them into allBadEntities
	}
	cout << "b" << endl;

	while (!toSearch.empty())  //while toSearch contains strings
	{
		cout << "c" << endl;
		string searchingFor = toSearch.front();
		toSearch.pop();
		cout << "D" << endl;
		findInteractions(searchingFor);  //map the set of entities that interact with toSearch.front
		set<string>::iterator it;  //search through every interactor entity
		it = InteractionMap[searchingFor].begin();
		cout << "e" << endl;

		while (it != InteractionMap[searchingFor].end())
		{
			cout << "f" << endl;
			string interactedWith = *it;
			findInteractions(interactedWith);  //map every interactor's own set of entities it interacts with to it in InteractionMap
			cout << "g" << endl;
			set<string> interactors = InteractionMap[interactedWith];  //gets the set of interactors for that entity
			cout << "h" << endl;
			if (PrevalenceMap[interactedWith] < minPrevalenceToBeGood)  //if that interactor has a prevalence less than the prevalence threshold, it is malicious
			{
				cout << "i" << endl;
				vector<string>::iterator begin, end, iter;
				begin = badEntitiesFound.begin();
				end = badEntitiesFound.end();
				iter = find(begin, end, interactedWith);  //look for the interactor in badEntitiesFound to check if it has already been found

				set<string>::iterator begin2, end2, iter2;
				begin2 = allBadEntities.begin();
				end2 = allBadEntities.end();
				iter2 = find(begin2, end2, interactedWith);
				
				if (iter2 == end2)  //if not found in allBadEntities, meaning never discovered before
				{
					badEntitiesFound.push_back(interactedWith);  //add it to badEntitiesFound
					allBadEntities.insert(interactedWith);  //add it to allBadEntities
					toSearch.push(interactedWith);  //add it to the toSearch queue
				}
				else if (iter == end)  //else, if found in allBadEntities but not badEntitiesFound
				{
					cout << "j" << endl;
					badEntitiesFound.push_back(interactedWith);  //add it to badEntitiesFound
				}
			}
			it++;
		}
	}
	cout << "k" << endl;
	//after searching through all the entities, time to search through all the interactions
	set<InteractionTuple>::iterator it;
	set<string>::iterator it2;
	cout << "afasFEFAFF" << endl;

	cout << "KKK" << endl;
	for (it = InteractionSet.begin(); it != InteractionSet.end(); ++it)
	{
		cout << "alskejfla" << endl;
		for (it2 = allBadEntities.begin(); it2 != allBadEntities.end(); ++it2)
		{
			cout << "ALEKFJA" << endl;
			InteractionTuple currTuple = *it;
			string toSearch = *it2;

			cout << "l" << endl;
			if (currTuple.from == toSearch || currTuple.to == toSearch);
			{
				badInteractions.push_back(currTuple);
				cout << "m" << endl;
			}
		}
	}
	vector<InteractionTuple>::iterator it3;
	for (it3 = badInteractions.begin(); it3 != badInteractions.end(); ++it3)
	{
		InteractionTuple t = *it3;
		cout << t.context << endl;
		cout << t.from << endl;
		cout << t.to << endl;
	}
	return allBadEntities.size();
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
		++forwardIt;
	}
	while (reverseIt.isValid())
	{
		MultiMapTuple rTuple = *reverseIt;
		bool rsuccess = reverseMap.erase(rTuple.key, rTuple.value, rTuple.context);
		if (rsuccess)
			result = true;
		++reverseIt;
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

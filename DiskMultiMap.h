#ifndef DISKMULTIMAP_H_
#define DISKMULTIMAP_H_

#include <string>
#include "MultiMapTuple.h"
#include "BinaryFile.h"

class DiskMultiMap
{
public:

	class Iterator
	{
	public:
		Iterator();
		// You may add additional constructors
		bool isValid() const;
		Iterator& operator++();
		MultiMapTuple operator*();

	private:
		// Your private member declarations will go here
	};

	DiskMultiMap();
	~DiskMultiMap();
	bool createNew(const std::string& filename, unsigned int numBuckets);
	bool openExisting(const std::string& filename);
	void close();
	bool insert(const std::string& key, const std::string& value, const std::string& context);
	Iterator search(const std::string& key);
	int erase(const std::string& key, const std::string& value, const std::string& context);

private:
	static const int MAX_STRING_SIZE = 120;
	static const int NULL_OFFSET = 0;

	struct Header
	{
		int numBuckets;
		BinaryFile::Offset spaceHead;
	};

	struct Bucket
	{
		BinaryFile::Offset head;
	};

	struct Node
	{
		char key[MAX_STRING_SIZE];
		char value[MAX_STRING_SIZE];
		char context[MAX_STRING_SIZE];
		BinaryFile::Offset next;
	};

	bool stringsMatch(Node n, const std::string& key, const std::string& value, const std::string& context);
	BinaryFile::Offset findBucketOffset(const std::string& key);

	BinaryFile bf;
};

#endif // DISKMULTIMAP_H_

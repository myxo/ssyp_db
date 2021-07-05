#include "storage.h"

class Storage : public IStorage {
	bool WriteToJournal(std::vector<std::string> ops) override {
		for (auto it : ops) {
			journal.push_back(it);
		}
		return true;
	}
    JournalBlob GetJournal() override {
        return journal;
    }
private:
	std::vector<std::string> journal;
};
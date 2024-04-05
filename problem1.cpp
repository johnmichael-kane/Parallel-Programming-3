#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <memory>
#include <random>

struct Present {
    int tag;
    std::shared_ptr<Present> next;

    Present(int t) : tag(t), next(nullptr) {}
};

class ConcurrentLinkedList {
private:
    std::shared_ptr<Present> head;
    std::mutex list_mutex;

public:
    ConcurrentLinkedList() : head(nullptr) {}

    void addPresent(int tag) {
        std::lock_guard<std::mutex> lock(list_mutex);
        auto newPresent = std::make_shared<Present>(tag);
        if (!head || tag < head->tag) {
            newPresent->next = head;
            head = newPresent;
        } else {
            auto current = head;
            while (current->next && current->next->tag < tag) {
                current = current->next;
            }
            newPresent->next = current->next;
            current->next = newPresent;
        }
        std::cout << "Present #" << tag << " added." << std::endl;
    }

    bool removePresent(int tag) {
        std::lock_guard<std::mutex> lock(list_mutex);
        auto current = head;
        std::shared_ptr<Present> prev = nullptr;

        while (current && current->tag != tag) {
            prev = current;
            current = current->next;
        }

        if (current) {
            if (prev) {
                prev->next = current->next;
            } else {
                head = current->next;
            }
            current->next = nullptr;
            return true;
        }
        return false;
    }

    bool findPresent(int tag) {
        std::lock_guard<std::mutex> lock(list_mutex);
        auto current = head;
        while (current) {
            if (current->tag == tag) {
                return true;
            }
            current = current->next;
        }
        return false;
    }
};

void writeThankYouNote(ConcurrentLinkedList& list, int tag) {
    if (list.removePresent(tag)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::cout << "Thank you " << tag << "!" << std::endl;
    }
}

void ServantWork(ConcurrentLinkedList& list, int start, int end) {
    for (int tag = start; tag <= end; ++tag) {
        list.addPresent(tag);
    }
    for (int tag = start; tag <= end; ++tag) {
        writeThankYouNote(list, tag);
    }
}

int main() {
    ConcurrentLinkedList presentsList;
    std::vector<std::thread> servants;
    int num_servants = 4;
    int presents = 100;
    
    int presents_per_servant = (presents + num_servants - 1) / num_servants;
    
    for (int i = 0; i < num_servants; ++i) {
        int start = i * presents_per_servant;
        int end = std::min(start + presents_per_servant - 1, presents - 1);
        servants.emplace_back(ServantWork, std::ref(presentsList), start, end);
    }

    for (auto& servant : servants) {
        servant.join();
    }

    std::cout << "All 'Thank You' notes have been written." << std::endl;
    return 0;
}
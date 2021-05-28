#include <unordered_map>
#include <mutex>
#include <utility>
#include <condition_variable>   

template<class K, class T>
class TS_unordered_multimap {
private:
    std::mutex mut;
    std::condition_variable condition;      
public:
    std::unordered_multimap<K, T> data; //in your case change this to a std::map

    typedef decltype(data.begin()) iterator; 
    
    TS_unordered_multimap(){}
    TS_unordered_multimap& operator = (TS_unordered_multimap&) = delete;

    void insert(std::pair<K, T> value){
        std::lock_guard<std::mutex> lk(mut);
        data.insert(value);
        condition.notify_one(); //if you have many threads trying to access the data at same time, this will wake one thread only
    }

    void erase(iterator it) {
        std::lock_guard<std::mutex> lk(mut);
        data.erase(it);
        condition.notify_one(); //if you have many threads trying to access the data at same time, this will wake one thread only
    }

    iterator find(K key){
        std::unique_lock<std::mutex> lk(mut);
        auto value = data.find(key);
        lk.unlock();
        return value;
    }
};
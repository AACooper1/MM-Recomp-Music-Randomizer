#include "common.hpp"

#include "seed/category.hpp"
#include "seed/songslot.hpp"

class Track;

extern std::unordered_map<std::string, std::vector<SongSlotID>> OoTBGMGroupsToCategories;
extern std::unordered_map<std::string, std::vector<SongSlotID>> OoTFanfareGroupsToCategories;

class Config {
    public:
        Config();
        Config(int trackId) : Config() { this->trackId = trackId; }

        int trackId;
        
        std::unique_ptr<std::vector<char>> categories;
        float volume = 100.0f;
        float weight = 1.0f;

        u8 is_songtest = false;
        u8 is_songforce = false;

        void parse_categories(std::vector<char>& filebuffer);
        void parse_categories(const std::vector<int>& categories); // Vanilla track overload
        void parse_oot_categories(std::string metaCategories);
    protected:
        std::shared_ptr<Track> track;

        bool _is_fanfare = false;
};
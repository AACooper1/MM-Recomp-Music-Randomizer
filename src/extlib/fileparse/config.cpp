#include "config.hpp"

Config::Config()
{
    categories = std::make_unique<std::vector<char>>(0x200, false);
    volume = 100.0f;
    weight = 1.0f;
}

void Config::parse_oot_categories(std::string metaCategories)
{
    _is_fanfare = metaCategories.contains("fanfare") || metaCategories.contains("Fanfare");

    if (metaCategories.length() == 0 || metaCategories == "bgm" || metaCategories == "bgm\n")
    {
          (*this->categories)[FIELD]
        = (*this->categories)[TOWN]
        = (*this->categories)[DUNGEON]
        = (*this->categories)[BUILDING]
        = (*this->categories)[MINIGAME]
        = true;
        return;
    }
    else if (metaCategories == "fanfare" || metaCategories == "fanfare\n")
    {
          (*this->categories)[FANFARE]
        = (*this->categories)[GAME_OVER]
        = (*this->categories)[AREA_CLEAR]
        = true;
        return;
    }
    else
    {
        std::vector<std::string> cats = split_string(metaCategories, ",\n");

        for (int i = 0; i < cats.size(); i++)
        {
            if (OoTBGMGroupsToCategories.contains(cats[i]))
            {
                std::vector<SongSlotID>& this_category = OoTBGMGroupsToCategories.at(cats[i]);
                for(int c = 0; c < this_category.size(); c++)
                {
                    (*this->categories)[(int)this_category[c]] = true;
                }
            }
        }
    }
}

void Config::parse_categories(std::vector<char>& filebuffer)
{
    std::string cats_txt(filebuffer.begin(), filebuffer.end());

    std::vector<std::string> categories = split_string(cats_txt, ",-");

    for (int i = 0; i < categories.size(); i++)
    {
        int cat = -1;
        try 
        {
            cat = std::stoi(categories[i], 0, 16);
            if (cat == 0x10) { cat = 10; }  // Special handling for these two, as MMR handles
            if (cat == 0x16) {cat = 16; }   // song slots in base 16 but categories in base 10
        }
        catch (const std::exception& e) 
        {
            logger.error << "Could not parse int for category \"" << categories[i] << "\"" << std:: endl;
            continue;
        }

        if (cat < 0x200)
        {
            (*this->categories)[cat] = true;
        }
        else
        {
            logger.warning << "Track had category " << cat << ", which is not allowed." << std::endl;
        }
    }

    return;
}


/* Used for vanilla tracks */
void Config::parse_categories(const std::vector<int>& categories)
{
    for (int i = 0; i < categories.size(); i++)
    {
        if (categories[i] < 0x200)
        {
            (*this->categories)[categories[i]] = true;
        }
        else
        {
            logger.warning << "Vanila track had category " << categories[i] << ", which is not allowed." << std::endl;
        }
    }
    (*this->categories)[std::abs(trackId)] = true;
}


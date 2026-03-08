#include <formmask.hpp>

void FormMask::parse_file(std::vector<char>& filebuffer)
{
    std::string formmask_txt(filebuffer.begin(), filebuffer.end());

    std::fill_n(states, 16, 0x0000);

    std::vector<std::string> channels = split_string(formmask_txt, "[\"]");
    for (int i = channels.size() - 1; i >= 0; i--)
    {
        bool is_real_line = false;
        for (int j = 0; j < channels[i].size(); j++)
        {
            if (std::isalpha(channels[i][j]))
            {
                is_real_line = true; 
                break;
            }
        }
        if (!is_real_line)
        {
            channels.erase(channels.begin()+i);
        }
    }
    if (channels.size() > 17)
    {
        logger.error << "Formmask has more than 17 rows " << "(" << channels.size() << "), skipping!";
    }

    for (int ch = 0; ch < channels.size() && ch < 16; ch++)
    {
        std::string& line = channels[ch];
        int stateIdx = 0;

        if (line.contains("None"))
        {
            stateIdx = 5;
        }
        else if (line.contains("All"))
        {
            states[ch] += FIERCE_DEITY | GORON | ZORA | DEKU | HUMAN;
            stateIdx = 5;
        }

        for (stateIdx; stateIdx < 15; stateIdx++)
        {
            if (line.contains(playStates[stateIdx]))
                states[ch] += (1 << stateIdx);
        }
    }

    if (channels.size() == 17)
    {
        std::string& line = channels[16];

        for (int stateIdx = 5; stateIdx < 12; stateIdx++)
        {
            if (line.contains(playStates[stateIdx]))
                cumulativeStates += (1 << stateIdx);
        }
    }

    for (int c = 5; c < 15; c++)
    {
        if (!formmask_txt.contains(playStates[c]))
        {
            logger.debug << playStates[c] << " not in formmask, will be added as cumulative state..." << std::endl;
            cumulativeStates += (1 << c);
        }
    }

    return;
}
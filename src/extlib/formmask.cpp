
void Track::parse_formmask(std::vector<char> filebuffer)
{
    std::string formmask_txt(filebuffer.begin(), filebuffer.end());

    std::vector<std::string> channels = split_string(formmask_txt, "[\"]");
    if (channels.size() > 16)
    {
        logger.error << "Formmask for " << name << " has more than 16 rows " << "(" << channels.size() << "), skipping!";
    }

    std::string playStates[13] = 
    {
        "FierceDeity",
        "Goron",
        "Zora",
        "Deku",
        "Human",
        "Outdoors",
        "Indoors",
        "Cave",
        "Epona",
        "Swim",
        "SpikeRolling",
        "Combat",
        "CriticalHealth"
    };
}